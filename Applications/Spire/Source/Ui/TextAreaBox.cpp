#include "Spire/Ui/TextAreaBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/ChainExpression.hpp"
#include "Spire/Styles/LinearExpression.hpp"
#include "Spire/Styles/RevertExpression.hpp"
#include "Spire/Styles/TimeoutExpression.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"


#include <QPlainTextEdit>
#include <QApplication>

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF, 0))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(LineHeight(1.25)).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor::fromRgb(0, 0, 0)));
    style.get(Any() >> is_a<Box>()).
      set(border_size(0));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0))).
      set(horizontal_padding(0));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0)));
    return style;
  }
}

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
}

TextAreaBox::TextAreaBox(QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(), parent) {}

TextAreaBox::TextAreaBox(QString current, QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(std::move(current)),
      parent) {}

TextAreaBox::TextAreaBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent),
      m_text_edit_styles{[=] { commit_style(); }},
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_longest_line_length(0),
      m_longest_line_block(0) {
  m_text_edit = new QTextEdit(this);
  m_text_edit->setAcceptRichText(false);
  m_text_edit->document()->setDocumentMargin(0);
  m_text_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_text_edit->setFrameShape(QFrame::NoFrame);
  connect(m_text_edit->document()->documentLayout(),
    &QAbstractTextDocumentLayout::documentSizeChanged, this,
    &TextAreaBox::on_document_size);
  connect(m_text_edit->document(), &QTextDocument::contentsChange, this,
    &TextAreaBox::on_contents_changed);
  setFocusProxy(m_text_edit);
  m_text_edit->installEventFilter(this);
  m_scroll_box = new ScrollBox(m_text_edit, this);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER,
    ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_scroll_box->setFocusPolicy(Qt::NoFocus);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  proxy_style(*this, *m_scroll_box);
  connect_style_signal(*this, [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_text_edit, &QTextEdit::cursorPositionChanged, this,
    &TextAreaBox::on_cursor_position);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) { on_current(value); });
}

const std::shared_ptr<TextModel>& TextAreaBox::get_model() const {
  return m_model;
}

const QString& TextAreaBox::get_submission() const {
  return m_submission;
}

bool TextAreaBox::is_read_only() const {
  return m_text_edit->isReadOnly();
}

void TextAreaBox::set_read_only(bool read_only) {
  m_text_edit->setReadOnly(read_only);
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  return {std::max(scale_width(1),
    m_longest_line_length + 2 + m_text_edit->cursorWidth()),
    line_count() * m_line_height + 2};
}

void TextAreaBox::mousePressEvent(QMouseEvent* event) {
  m_text_edit->setFocus();
  QWidget::mousePressEvent(event);
}

void TextAreaBox::resizeEvent(QResizeEvent* event) {
  update_text_width();
  QWidget::resizeEvent(event);
}

void TextAreaBox::commit_style() {
  auto stylesheet = QString(
    R"(QTextEdit {
      background: transparent;
      border-width: 0px;
      padding: 0px;)");
  m_text_edit_styles.m_styles.write(stylesheet);
  auto alignment = m_text_edit_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  if(alignment != m_text_edit->alignment()) {
    auto cursor_pos = m_text_edit->textCursor().position();
    for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
      auto block = m_text_edit->document()->findBlockByNumber(i);
      if(block.isValid()) {
        auto cursor = m_text_edit->textCursor();
        cursor.setPosition(block.position());
        m_text_edit->setTextCursor(cursor);
        m_text_edit->setAlignment(alignment);
      }
    }
    auto cursor = m_text_edit->textCursor();
    cursor.setPosition(cursor_pos);
    m_text_edit->setTextCursor(cursor);
  }
  auto font = m_text_edit_styles.m_font.value_or(QFont());
  if(m_text_edit_styles.m_size) {
    font.setPixelSize(*m_text_edit_styles.m_size);
  }
  m_text_edit->setFont(font);
  if(m_text_edit_styles.m_line_height && ((static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height) != m_line_height)) {
    m_line_height = static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height;
    m_scroll_box->get_vertical_scroll_bar().set_line_size(m_line_height);
    auto cursor_pos = m_text_edit->textCursor().position();
    for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
      auto block = m_text_edit->document()->findBlockByNumber(i);
      if(block.isValid()) {
        auto cursor = m_text_edit->textCursor();
        cursor.setPosition(block.position());
        m_text_edit->setTextCursor(cursor);
        auto block_format = cursor.blockFormat();
        block_format.setLineHeight(m_line_height,
          QTextBlockFormat::FixedHeight);
        cursor.setBlockFormat(block_format);
        m_text_edit->setTextCursor(cursor);
      }
    }
    auto cursor = m_text_edit->textCursor();
    cursor.setPosition(cursor_pos);
    m_text_edit->setTextCursor(cursor);
  }
  if(stylesheet != m_text_edit->styleSheet()) {
    m_text_edit->setStyleSheet(stylesheet);
    m_text_edit->style()->unpolish(this);
    m_text_edit->style()->polish(this);
  }
}

QSize TextAreaBox::compute_decoration_size() const {
  auto decoration_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*m_scroll_box)) {
    property.visit(
      [&] (std::in_place_type_t<BorderTopSize>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<BorderBottomSize>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<PaddingTop>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingRight>, int size) {
        decoration_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<PaddingBottom>, int size) {
        decoration_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingLeft>, int size) {
        decoration_size.rwidth() += size;
      });
  }
  return decoration_size;
}

int TextAreaBox::get_text_length(const QString& text) {
  return m_text_edit->fontMetrics().horizontalAdvance(text) +
    m_text_edit->cursorWidth();
}

bool TextAreaBox::is_scroll_bar_visible() const {
  return line_count() > visible_line_count();
}

int TextAreaBox::line_count() const {
  auto num = 0;
  for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
    auto block = m_text_edit->document()->findBlockByNumber(i);
    if(block.isValid()) {
      num += block.layout()->lineCount();
    }
  }
  return std::max(1, num);
}

void TextAreaBox::update_text_width() {
  auto scroll_bar_width = [&] {
    if(is_scroll_bar_visible()) {
      return scale_width(15);
    }
    return 0;
  }();
  m_text_edit->document()->setTextWidth(std::min(width() - 2,
    m_longest_line_length) + m_text_edit->cursorWidth() - scroll_bar_width);
  updateGeometry();
}

int TextAreaBox::visible_line_count() const {
  return height() / m_line_height;
}

void TextAreaBox::on_contents_changed(int position, int removed, int added) {
  auto block = m_text_edit->document()->findBlock(position);
  if(block.isValid()) {
    auto line_length = get_text_length(block.text());
    if(line_length > m_longest_line_length) {
      m_longest_line_length = line_length;
      m_longest_line_block = block.blockNumber();
    } else if(block.blockNumber() == m_longest_line_block) {
      m_longest_line_length = 0;
      m_longest_line_block = 0;
      for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
        auto block = m_text_edit->document()->findBlockByNumber(i);
        if(block.isValid()) {
          if(get_text_length(block.text()) > m_longest_line_length) {
            m_longest_line_length = get_text_length(block.text());
            m_longest_line_block = block.blockNumber();
          }
        }
      }
    }
  }
  update_text_width();
}

void TextAreaBox::on_current(const QString& current) {

}

void TextAreaBox::on_cursor_position() {
  auto top = m_text_edit->visibleRegion().boundingRect().top();
  auto bottom = m_text_edit->visibleRegion().boundingRect().bottom();
  if(m_scroll_box->get_vertical_scroll_bar().isVisible() &&
      !m_text_edit->visibleRegion().boundingRect().contains(
        m_text_edit->cursorRect())) {
    if(m_text_edit->cursorRect().top() <= top) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().top() - scale_height(8));
    } else if(m_text_edit->cursorRect().bottom() >= bottom) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().bottom() -
        m_text_edit->visibleRegion().boundingRect().height() +
        scale_height(8));
    }
  }
}

void TextAreaBox::on_document_size(const QSizeF& size) {
  m_text_edit->setFixedSize(size.toSize());
  updateGeometry();
}

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  m_text_edit_styles.clear();
  m_text_edit_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const TextColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_text_edit_styles.m_styles.set("color", color);
          });
        },
        [&] (const TextAlign& alignment) {
          stylist.evaluate(alignment, [=] (auto alignment) {
            m_text_edit_styles.m_alignment = alignment;
          });
        },
        [&] (const Font& font) {
          stylist.evaluate(font, [=] (const auto& font) {
            m_text_edit_styles.m_font = font;
          });
        },
        [&] (const FontSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_size = size;
          });
        },
        [&] (const LineHeight& height) {
          stylist.evaluate(height, [=] (auto height) {
            m_text_edit_styles.m_line_height = height;
          });
        });
    }
  });
}
