#include "Spire/Ui/TextAreaBox.hpp"
#include <QAbstractTextDocumentLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto ELLIPSES_CHAR = 0x2026;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Normal);
    font.setPixelSize(scale_width(12));
    style.get(Any()).
      set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(LineHeight(1.20)).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft))).
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextColor(QColor::fromRgb(0, 0, 0))).
      set(horizontal_padding(8)).
      set(vertical_padding(5));
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
    style.get(Placeholder()).set(TextColor(QColor::fromRgb(0xA0, 0xA0, 0xA0)));
    style.get(Disabled() / Placeholder()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

TextAreaBox::ContentSizedTextEdit::ContentSizedTextEdit(const QString& text,
    QWidget* parent)
    : QTextEdit(parent) {
  setLineWrapMode(QTextEdit::WidgetWidth);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  document()->setDocumentMargin(0);
  setFrameShape(QFrame::NoFrame);
  setAcceptRichText(false);
  connect(
    this, &QTextEdit::textChanged, this, [=] { on_text_changed(); });
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setText(text);
}

QSize TextAreaBox::ContentSizedTextEdit::sizeHint() const {
  auto margins = contentsMargins();
  auto desired_size = QSize(m_longest_line_width,
    document()->size().toSize().height());
  auto size = desired_size + QSize(
    margins.left() + margins.right(), margins.top() +
    margins.bottom());
  if(!isReadOnly()) {
    size.rwidth() += cursorWidth();
  }
  return size;
}

QSize TextAreaBox::ContentSizedTextEdit::minimumSizeHint() const {
  return QSize();
}

void TextAreaBox::ContentSizedTextEdit::on_text_changed() {
  m_longest_line_width = get_longest_line_width();
  updateGeometry();
}

int TextAreaBox::ContentSizedTextEdit::get_longest_line_width() const {
  auto longest = 0;
  for(auto i = 0; i < document()->blockCount(); ++i) {
    auto block = document()->findBlockByNumber(i);
    if(block.isValid()) {
      longest = std::max(longest,
        fontMetrics().horizontalAdvance(block.text()));
    }
  }
  return longest;
}

TextAreaBox::ElidedLabel::ElidedLabel(QWidget* parent)
  : QWidget(parent) {}

void TextAreaBox::ElidedLabel::set_text(const QString &text) {
  m_text = text;
  update();
}

void TextAreaBox::ElidedLabel::set_text_color(const QColor& color) {
  m_text_color = color;
  update();
}

void TextAreaBox::ElidedLabel::set_alignment(Qt::Alignment alignment) {
  m_alignment = alignment;
  update();
}

void TextAreaBox::ElidedLabel::set_line_height(int line_height) {
  m_line_height = line_height;
  update();
}

QSize TextAreaBox::ElidedLabel::sizeHint() const  {
  return {width(), height()};
}

void TextAreaBox::ElidedLabel::paintEvent(QPaintEvent *event) {
  auto painter = QPainter(this);
  painter.setPen(m_text_color);
  painter.setFont(font());
  auto fontMetrics = painter.fontMetrics();
  auto y = m_line_height - fontMetrics.height();
  auto textLayout = QTextLayout(m_text, painter.font());
  auto opt = textLayout.textOption();
  opt.setAlignment(m_alignment);
  textLayout.setTextOption(opt);
  textLayout.beginLayout();
  auto line = textLayout.createLine();
  while(line.isValid()) {
    line.setLineWidth(width());
    auto next_line_y = y + m_line_height;
    if (height() >= next_line_y + m_line_height) {
      line.draw(&painter, QPoint(0, y));
      y = next_line_y;
    } else {
      auto last_line = m_text.mid(line.textStart());
      auto elided_last_line = fontMetrics.elidedText(last_line,
        Qt::ElideRight, width());
      painter.drawText(QPoint(0, y + fontMetrics.ascent()),
        elided_last_line);
      line = textLayout.createLine();
      break;
    }
    line = textLayout.createLine();
  }
  textLayout.endLayout();
}

void TextAreaBox::StyleProperties::clear() {
  m_styles.clear();
  m_alignment = none;
  m_font = none;
  m_size = none;
  m_color = none;
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
      m_submission(m_model->get_current()) {
  m_text_edit = new ContentSizedTextEdit(m_model->get_current(), this);
  m_text_edit->installEventFilter(this);
  m_stacked_widget = new QStackedWidget(this);
  m_stacked_widget->addWidget(m_text_edit);
  setFocusProxy(m_text_edit);
  connect(m_text_edit->document(), &QTextDocument::contentsChanged, this,
    &TextAreaBox::on_text_changed);
  m_placeholder = new ElidedLabel(this);
  m_placeholder->setFixedSize(0, 0);
  m_stacked_widget->addWidget(m_placeholder);
  m_placeholder->setCursor(m_text_edit->cursor());
  m_placeholder->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_scroll_box = new ScrollBox(m_stacked_widget, this);
  m_scroll_box->set(ScrollBox::DisplayPolicy::NEVER,
    ScrollBox::DisplayPolicy::ON_OVERFLOW);
  m_scroll_box->get_vertical_scroll_bar().adjustSize();
  m_scroll_box->installEventFilter(this);
  m_scroll_box->findChild<ScrollableLayer*>()->installEventFilter(this);
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
  m_text_edit->setReadOnly(read_only);
  m_scroll_box->get_vertical_scroll_bar().set_position(0);
  if(read_only) {
    m_scroll_box->set_vertical(ScrollBox::DisplayPolicy::NEVER);
    match(*this, ReadOnly());
  } else {
    m_scroll_box->set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
    unmatch(*this, ReadOnly());
  }
  update_display_text();
  update_placeholder_text();
  update_text_edit_width();
}

connection
    TextAreaBox::connect_submit_signal(const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextAreaBox::sizeHint() const {
  return m_text_edit->sizeHint() + compute_border_size() +
    compute_padding_size();
}

void TextAreaBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_placeholder_text();
  }
  QWidget::changeEvent(event);
}

bool TextAreaBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_scroll_box && event->type() == QEvent::Resize) {
    update_text_edit_width();
    m_stacked_widget->setMinimumSize(size() - compute_border_size() -
      compute_padding_size());
    m_stacked_widget->adjustSize();
    update_display_text();
    update_placeholder_text();
  } else if(watched == m_text_edit && event->type() == QEvent::FocusOut) {
    m_submit_signal(m_model->get_current());
  } else if(event->type() == QEvent::Wheel) {
    if(m_text_edit->isReadOnly()) {
      return true;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TextAreaBox::mousePressEvent(QMouseEvent* event) {
  m_text_edit->setFocus();
  QWidget::mousePressEvent(event);
}

void TextAreaBox::commit_placeholder_style() {
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
    update_text_alignment();
  }
  auto font = m_text_edit_styles.m_font.value_or(QFont());
  if(m_text_edit_styles.m_size) {
    font.setPixelSize(*m_text_edit_styles.m_size);
  }
  m_text_edit->setFont(font);
  if(m_text_edit_styles.m_line_height &&
      (font.pixelSize() * *m_text_edit_styles.m_line_height) !=
      m_computed_line_height) {
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

QSize TextAreaBox::compute_border_size() const {
  auto border_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*this)) {
    property.visit(
      [&] (std::in_place_type_t<BorderTopSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        border_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<BorderBottomSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        border_size.rwidth() += size;
      });
  }
  return border_size;
}

QSize TextAreaBox::compute_padding_size() const {
  auto padding_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*this)) {
    property.visit(
      [&] (std::in_place_type_t<PaddingTop>, int size) {
        padding_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingRight>, int size) {
        padding_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<PaddingBottom>, int size) {
        padding_size.rheight() += size;
      },
      [&] (std::in_place_type_t<PaddingLeft>, int size) {
        padding_size.rwidth() += size;
      });
  }
  return padding_size;
}

void TextAreaBox::update_display_text() {
  if(is_read_only()) {
    if(m_text_edit->toPlainText() != m_model->get_current()) {
      m_text_edit->blockSignals(true);
      m_text_edit->setText(m_model->get_current());
      m_text_edit->blockSignals(false);
    }
    auto line_count = std::floor(static_cast<double>((height() -
      compute_padding_size().height())) /
      static_cast<double>(m_computed_line_height));
    if(line_count > 0) {
      auto lines = [&] {
        auto lines = QStringList();
        auto block = m_text_edit->document()->begin();
        while(block.isValid()) {
          auto block_text = block.text();
          if(!block.layout()) {
            continue;
          }
          for(int i = 0; i != block.layout()->lineCount(); ++i) {
            auto line = block.layout()->lineAt(i);
            lines.append(block_text.mid(line.textStart(), line.textLength()));
          }
          if(lines.count() > line_count) {
            break;
          }
          block = block.next();
        }
        return lines;
      }();
      if(lines.count() > line_count) {
        auto is_elided = lines.count() > line_count;
        while(lines.count() > line_count) {
          lines.pop_back();
        }
        auto& last_line = lines.back();
        last_line = m_text_edit->fontMetrics().elidedText(last_line,
          Qt::ElideRight, width() - compute_border_size().width() -
          compute_padding_size().width() -
          m_text_edit->fontMetrics().horizontalAdvance(ELLIPSES_CHAR));
        if(is_elided && !last_line.endsWith(ELLIPSES_CHAR)) {
          last_line.append(ELLIPSES_CHAR);
        }
        m_text_edit->blockSignals(true);
        m_text_edit->setText(lines.join("\n"));
        m_text_edit->blockSignals(false);
      }
    }
    update_text_alignment();
    update_document_line_height();
    return;
  } else if(m_text_edit->toPlainText() != m_model->get_current()) {
    m_text_edit->setText(m_model->get_current());
  }
  update_text_alignment();
  update_document_line_height();
}

void TextAreaBox::update_document_line_height() {
  auto cursor_pos = m_text_edit->textCursor().position();
  for(auto i = 0; i < m_text_edit->document()->blockCount(); ++i) {
    auto block = m_text_edit->document()->findBlockByNumber(i);
    if(block.isValid()) {
      auto cursor = m_text_edit->textCursor();
      cursor.setPosition(block.position());
      m_text_edit->setTextCursor(cursor);
      auto block_format = cursor.blockFormat();
      block_format.setLineHeight(m_computed_line_height,
        QTextBlockFormat::FixedHeight);
      cursor.setBlockFormat(block_format);
      m_text_edit->setTextCursor(cursor);
    }
  }
  auto cursor = m_text_edit->textCursor();
  cursor.setPosition(cursor_pos);
  m_text_edit->setTextCursor(cursor);
}

void TextAreaBox::update_line_height() {
  m_computed_line_height =
    static_cast<int>(static_cast<double>(m_text_edit->font().pixelSize()) *
    *m_text_edit_styles.m_line_height);
  m_placeholder->set_line_height(m_computed_line_height);
  m_scroll_box->get_vertical_scroll_bar().set_line_size(
    m_computed_line_height);
  update_display_text();
}

void TextAreaBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    m_placeholder->set_text(m_placeholder_text);
    m_placeholder->setFixedSize(size() - compute_border_size() -
      compute_padding_size());
    m_stacked_widget->adjustSize();
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}

void TextAreaBox::update_text_alignment() {
  auto alignment = m_text_edit_styles.m_alignment.value_or(Qt::AlignLeft);
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
  auto border_size = compute_border_size();
  auto padding_size = compute_padding_size();
  if(m_text_edit->document()->size().toSize().height() > height() -
      padding_size.height() - border_size.height() &&
      !m_text_edit->isReadOnly()) {
    m_text_edit->setFixedWidth(width() -
      m_scroll_box->get_vertical_scroll_bar().width() - border_size.width() -
      padding_size.width());
  } else {
    m_text_edit->setFixedWidth(width() - border_size.width() -
      padding_size.width());
  }
}

void TextAreaBox::on_current(const QString& current) {
  if(m_text_edit->toPlainText() != current) {
    m_text_edit->setText(current);
    update_display_text();
  }
}

void TextAreaBox::on_cursor_position() {
  if(!m_text_edit->hasFocus()) {
    return;
  }
  if(m_scroll_box->get_vertical_scroll_bar().isVisible() &&
      !m_text_edit->visibleRegion().boundingRect().contains(
        m_text_edit->cursorRect())) {
    auto top = m_text_edit->visibleRegion().boundingRect().top();
    auto bottom = m_text_edit->visibleRegion().boundingRect().bottom();
    if(m_text_edit->cursorRect().top() <= top) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().top());
    } else if(m_text_edit->cursorRect().bottom() >= bottom) {
      m_scroll_box->get_vertical_scroll_bar().set_position(
        m_text_edit->cursorRect().bottom() -
        m_text_edit->visibleRegion().boundingRect().height() +
        compute_padding_size().height());
    }
  }
}

void TextAreaBox::on_style() {
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
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
  auto& placeholder_stylist = *find_stylist(*this, Placeholder());
  merge(block, placeholder_stylist.get_computed_block());
  m_placeholder_styles.clear();
  m_placeholder_styles.m_styles.buffer([&] {
    for(auto& property : block) {
      property.visit(
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
        },
        [&] (const LineHeight& height) {
          stylist.evaluate(height, [=] (auto height) {
            m_placeholder_styles.m_line_height = height;
          });
        });
    }
  });
}

void TextAreaBox::on_text_changed() {
  if(!is_read_only() && m_text_edit->toPlainText() != m_model->get_current()) {
    m_model->set_current(m_text_edit->toPlainText());
  }
  update_text_edit_width();
  m_stacked_widget->adjustSize();
  updateGeometry();
  update_placeholder_text();
}
