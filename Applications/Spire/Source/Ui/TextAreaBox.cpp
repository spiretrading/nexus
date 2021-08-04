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
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

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
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      // TODO: 1.24 gives correct intrinsic size, spec change for calculated font height
      //        or impl change?
      set(LineHeight(1.24)).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor::fromRgb(0, 0, 0)));
    style.get(Any() >> is_a<QTextEdit>()).
      set(TextColor(QColor::fromRgb(0, 0, 0))).
      set(horizontal_padding(8)).
      set(vertical_padding(5));
    style.get(Hover() || Focus()).
      set(border_color(QColor::fromRgb(0x4B, 0x23, 0xA0)));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0)));
    style.get(ReadOnly() >> is_a<QTextEdit>()).
      set(horizontal_padding(0));
    style.get(Disabled()).
      set(BackgroundColor(QColor::fromRgb(0xF5, 0xF5, 0xF5))).
      set(border_color(QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    style.get(ReadOnly() && Disabled()).
      set(BackgroundColor(QColor::fromRgb(0, 0, 0, 0))).
      set(border_color(QColor::fromRgb(0, 0, 0, 0)));
    style.get(Placeholder()).set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() / Placeholder()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_padding = {};
  m_alignment = none;
  m_font = none;
  m_size = none;
  m_color = none;
  m_line_height = none;
}

TextAreaBox::TextAreaBox(QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(), parent) {}

TextAreaBox::TextAreaBox(QString current, QWidget* parent)
  : TextAreaBox(std::make_shared<LocalTextModel>(std::move(current)),
      parent) {}

TextAreaBox::TextAreaBox(std::shared_ptr<TextModel> model, QWidget* parent)
    : QWidget(parent),
      m_placeholder_styles{[=] { commit_placeholder_style(); }},
      m_text_edit_styles{[=] { commit_style(); }},
      m_model(std::move(model)),
      m_submission(m_model->get_current()),
      m_longest_line_length(0),
      m_longest_line_block(0),
      m_is_read_only(false) {
  m_text_edit = new ContentSizedTextEdit(this);
  m_stacked_widget = new QStackedWidget(this);
  m_stacked_widget->addWidget(m_text_edit);
  setFocusProxy(m_text_edit);
  connect(m_text_edit->document(), &QTextDocument::contentsChanged, this,
    &TextAreaBox::on_text_changed);
  m_placeholder = new ElidedLabel("", this);
  m_placeholder->setFixedSize(0, 0);
  m_stacked_widget->addWidget(m_placeholder);
  m_placeholder->setCursor(m_text_edit->cursor());
  m_placeholder->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_scroll_box = new ScrollBox(m_stacked_widget, this);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER,
    ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->get_vertical_scroll_bar().adjustSize();
  m_scroll_box->installEventFilter(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_scroll_box);
  proxy_style(*this, *m_scroll_box);
  add_pseudo_element(*this, Placeholder());
  connect_style_signal(*this, [=] { on_style(); });
  connect_style_signal(*this, Placeholder(), [=] { on_style(); });
  set_style(*this, DEFAULT_STYLE());
  connect(m_text_edit, &QTextEdit::cursorPositionChanged, this,
    &TextAreaBox::on_cursor_position);
  m_current_connection = m_model->connect_current_signal(
    [=] (const auto& value) { on_current(value); });
  connect(m_text_edit->document()->documentLayout(),
    &QAbstractTextDocumentLayout::documentSizeChanged,
    [=] (const auto& size) { m_stacked_widget->adjustSize(); });
}

const std::shared_ptr<TextModel>& TextAreaBox::get_model() const {
  return m_model;
}

const QString& TextAreaBox::get_submission() const {
  return m_submission;
}

void TextAreaBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
  update_placeholder_text();
}

bool TextAreaBox::is_read_only() const {
  return m_text_edit->isReadOnly();
}

void TextAreaBox::set_read_only(bool read_only) {
  // TODO: if setReadOnly is called here, the read-only style is overwritten.
  m_is_read_only = read_only;
  if(read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
  m_text_edit->setReadOnly(read_only);
  update_display_text();
  update_placeholder_text();
  update_text_edit_width();
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  return m_text_edit->sizeHint() + QSize(2, 2);
}

void TextAreaBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    //update_display_text();
    update_placeholder_text();
  }
  QWidget::changeEvent(event);
}

bool TextAreaBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Resize && watched == m_scroll_box) {
    update_text_edit_width();
    m_stacked_widget->adjustSize();
    //update_display_text();
    update_placeholder_text();
  }
  return QWidget::eventFilter(watched, event);
}

void TextAreaBox::mousePressEvent(QMouseEvent* event) {
  m_text_edit->setFocus();
  QWidget::mousePressEvent(event);
}

void TextAreaBox::commit_placeholder_style() {
  m_placeholder->set_padding(m_placeholder_styles.m_padding);
  auto alignment = m_placeholder_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  m_placeholder->set_alignment(alignment);
  auto font = m_placeholder_styles.m_font.value_or(QFont());
  if(m_placeholder_styles.m_size) {
    font.setPixelSize(*m_placeholder_styles.m_size);
  }
  m_placeholder->setFont(font);
  m_placeholder->set_text_color(m_placeholder_styles.m_color.value_or(
    QColor()));
  update_placeholder_text();
}

void TextAreaBox::commit_style() {
  auto stylesheet = QString(
    R"(QTextEdit {
      background: transparent;
      border-width: 0px;)");
  m_text_edit_styles.m_styles.write(stylesheet);
  auto alignment = m_text_edit_styles.m_alignment.value_or(
    Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
  if(alignment != m_text_edit->alignment()) {
    update_text_alignment(alignment);
  }
  auto font = m_text_edit_styles.m_font.value_or(QFont());
  if(m_text_edit_styles.m_size) {
    font.setPixelSize(*m_text_edit_styles.m_size);
  }
  m_text_edit->setFont(font);
  if(m_text_edit_styles.m_line_height && ((static_cast<double>(font.pixelSize()) *
      *m_text_edit_styles.m_line_height) != m_line_height)) {
    update_line_height();
  }
  if(stylesheet != m_text_edit->styleSheet()) {
    m_text_edit->setStyleSheet(stylesheet);
    m_text_edit->style()->unpolish(this);
    m_text_edit->style()->polish(this);
  }
}

bool TextAreaBox::is_placeholder_shown() const {
  return !is_read_only() && m_model->get_current().isEmpty() &&
    !m_placeholder_text.isEmpty();
}

//std::vector<QString> TextAreaBox::get_current_text_lines() const {
//  auto lines = std::vector<QString>();
//  lines.reserve(m_text_edit->document()->blockCount());
//  for(auto i = m_text_edit->document()->begin();
//      i != m_text_edit->document()->end(); i = i.next()) {
//    lines.push_back(i.text());
//  }
//  return lines;
//}

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

void TextAreaBox::update_display_text() {
  if(is_read_only()) {
    auto line_count = std::floor(static_cast<double>((height() - 10 - 2)) /
      static_cast<double>(m_line_height));
    auto lines = [&] {
      QStringList ret;
      QTextBlock tb = m_text_edit->document()->begin();
      while(tb.isValid())
      {
        QString blockText = tb.text();
        Q_ASSERT(tb.layout());
        if(!tb.layout())
          continue;
        for(int i = 0; i != tb.layout()->lineCount(); ++i)
        {
          QTextLine line = tb.layout()->lineAt(i);
          ret.append(blockText.mid(line.textStart(), line.textLength()));
        }
        if(ret.count() > line_count) {
          break;
        }
        tb = tb.next();
      }
      return ret;
    }();
    if(lines.count() > line_count) {
      lines.pop_back();
      // TODO: elide if required
      m_text_edit->setText(lines.join("\n"));
      update_text_alignment(*m_text_edit_styles.m_alignment);
      update_line_height();
    }
  } else if(m_text_edit->toPlainText() != m_model->get_current()) {
    m_text_edit->setText(m_model->get_current());
    update_text_alignment(*m_text_edit_styles.m_alignment);
    update_line_height();
  }
}

void TextAreaBox::update_line_height() {
  m_line_height = static_cast<double>(m_text_edit->font().pixelSize()) *
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

void TextAreaBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    auto font_metrics = m_placeholder->fontMetrics();
    auto rect = QRect(QPoint(0, 0), size() - compute_decoration_size());
    //auto elided_text =
    //  font_metrics.elidedText(m_placeholder_text, Qt::ElideRight, rect.width());
    m_placeholder->set_text(m_placeholder_text);
    m_placeholder->setFixedSize(size() - QSize(2, 2));
    m_stacked_widget->adjustSize();
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}

void TextAreaBox::update_text_alignment(Qt::Alignment alignment) {
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

void TextAreaBox::update_text_edit_width() {
  if(m_text_edit->document()->size().toSize().height() + 10 > height() - 2 ||
      m_scroll_box->get_vertical_scroll_bar().isVisible()) {
    m_text_edit->setFixedWidth(width() -
      m_scroll_box->get_vertical_scroll_bar().width() - 2);
  } else {
    m_text_edit->setFixedWidth(width() - 2);
  }
}

//void TextAreaBox::on_contents_changed(int position, int removed, int added) {
//  auto block = m_text_edit->document()->findBlock(position);
//  if(block.isValid()) {
//    auto line_length = get_text_length(block.text());
//    if(line_length > m_longest_line_length) {
//      m_longest_line_length = line_length;
//      m_longest_line_block = block.blockNumber();
//    } else if(block.blockNumber() == m_longest_line_block) {
//      m_longest_line_length = 0;
//      m_longest_line_block = 0;
//      for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
//        auto block = m_text_edit->document()->findBlockByNumber(i);
//        if(block.isValid()) {
//          if(get_text_length(block.text()) > m_longest_line_length) {
//            m_longest_line_length = get_text_length(block.text());
//            m_longest_line_block = block.blockNumber();
//          }
//        }
//      }
//    }
//  }
//  update_text_width();
//}

void TextAreaBox::on_current(const QString& current) {
  if(m_text_edit->toPlainText() != current) {
    m_text_edit->setText(current);
  }
}

void TextAreaBox::on_cursor_position() {
  if(m_scroll_box->get_vertical_scroll_bar().isVisible() &&
      !m_text_edit->visibleRegion().boundingRect().contains(
        m_text_edit->cursorRect())) {
    auto top = m_text_edit->visibleRegion().boundingRect().top();
    auto bottom = m_text_edit->visibleRegion().boundingRect().bottom();
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

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  auto& text_edit_stylist = find_stylist(*m_text_edit);
  auto text_edit_block = text_edit_stylist.get_computed_block();
  merge(block, text_edit_block);
  m_text_edit_styles.clear();
  m_text_edit_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const PaddingTop& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_styles.set("padding-top", size);
          });
        },
        [&] (const PaddingRight& size) {
          stylist.evaluate(size, [=] (auto size) {
            // TODO: fix selector
            if(m_is_read_only) {
              size = 0;
              //qDebug() << "size zero";
            }
            m_text_edit_styles.m_styles.set("padding-right", size);
          });
        },
        [&] (const PaddingBottom& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_text_edit_styles.m_styles.set("padding-bottom", size);
          });
        },
        [&] (const PaddingLeft& size) {
          stylist.evaluate(size, [=] (auto size) {
            // TODO: fix selector
            if(m_is_read_only) {
              size = 0;
            }
            m_text_edit_styles.m_styles.set("padding-left", size);
          });
        },
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
  auto& placeholder_stylist = *find_stylist(*this, Placeholder());
  merge(block, placeholder_stylist.get_computed_block());
  m_placeholder_styles.clear();
  m_placeholder_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
        [&] (const PaddingTop& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_padding.setTop(size);
          });
        },
        [&] (const PaddingRight& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_padding.setRight(size);
          });
        },
        [&] (const PaddingBottom& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_padding.setBottom(size);
          });
        },
        [&] (const PaddingLeft& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_padding.setLeft(size);
          });
        },
        [&] (const TextColor& color) {
          placeholder_stylist.evaluate(color, [=] (auto color) {
            m_placeholder_styles.m_color = color;
          });
        },
        [&] (const TextAlign& alignment) {
          placeholder_stylist.evaluate(alignment, [=] (auto alignment) {
            m_placeholder_styles.m_alignment = alignment;
          });
        },
        [&] (const Font& font) {
          placeholder_stylist.evaluate(font, [=] (auto font) {
            m_placeholder_styles.m_font = font;
          });
        },
        [&] (const FontSize& size) {
          placeholder_stylist.evaluate(size, [=] (auto size) {
            m_placeholder_styles.m_size = size;
          });
        });
    }
  });
}

void TextAreaBox::on_text_changed() {
  if(!is_read_only()) {
    m_model->set_current(m_text_edit->toPlainText());
  }
  update_text_edit_width();
  updateGeometry();
  update_placeholder_text();
}
