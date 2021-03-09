#include "Spire/Ui/TextBox.hpp"
#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

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
      set(BackgroundColor(QColor::fromRgb(255, 255, 255))).
      set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8))).
      set(text_style(font, QColor::fromRgb(0, 0, 0))).
      set(TextAlign(Qt::Alignment(Qt::AlignLeft) | Qt::AlignVCenter)).
      set(horizontal_padding(scale_width(8)));
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
    style.get(Placeholder() && Disabled()).
      set(TextColor(QColor::fromRgb(0xC8, 0xC8, 0xC8)));
    return style;
  }
}

TextStyle Spire::Styles::text_style(QFont font, QColor color) {
  return TextStyle(Font(std::move(font)), TextColor(std::move(color)));
}

TextBox::TextBox(QWidget* parent)
  : TextBox({}, parent) {}

TextBox::TextBox(const QString& current, QWidget* parent)
    : StyledWidget(parent),
      m_current(current) {
  m_line_edit = new QLineEdit(m_current, this);
  m_line_edit->setFrame(false);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit->installEventFilter(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_line_edit);
  m_placeholder = new QLabel(this);
  m_placeholder->setCursor(m_line_edit->cursor());
  m_placeholder->setAttribute(Qt::WA_TranslucentBackground);
  m_placeholder->setTextFormat(Qt::PlainText);
  m_placeholder->setTextInteractionFlags(Qt::NoTextInteraction);
  m_placeholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_line_edit->stackUnder(m_placeholder);
  set_style(DEFAULT_STYLE());
  setFocusProxy(m_line_edit);
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
  connect(m_line_edit, &QLineEdit::textChanged, this,
    &TextBox::on_text_changed);
}

const QString& TextBox::get_current() const {
  return m_current;
}

void TextBox::set_current(const QString& value) {
  m_current = value;
  update_display_text();
}

const QString& TextBox::get_submission() const {
  return m_submission;
}

void TextBox::set_placeholder(const QString& value) {
  m_placeholder_text = value;
  update_placeholder_text();
}

void TextBox::set_read_only(bool read_only) {
  m_line_edit->setReadOnly(read_only);
  update_display_text();
  update_placeholder_text();
}

bool TextBox::is_read_only() const {
  return m_line_edit->isReadOnly();
}

connection TextBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection TextBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

void TextBox::style_updated() {
  update_display_text();
  update_placeholder_text();
  StyledWidget::style_updated();
}

bool TextBox::test_selector(const Styles::Selector& selector) const {
  return selector.visit(
    [&] (Placeholder) {
      return is_placeholder_shown();
    },
    [&] (ReadOnly) {
      return is_read_only();
    },
    [&] {
      return StyledWidget::test_selector(selector);
    });
}

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::FocusIn) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      m_line_edit->setText(m_current);
    }
  } else if(event->type() == QEvent::FocusOut) {
    auto focusEvent = static_cast<QFocusEvent*>(event);
    if(focusEvent->lostFocus() &&
        focusEvent->reason() != Qt::ActiveWindowFocusReason &&
        focusEvent->reason() != Qt::PopupFocusReason) {
      elide_text();
    }
  }
  return StyledWidget::eventFilter(watched, event);
}

void TextBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_display_text();
    update_placeholder_text();
  }
  StyledWidget::changeEvent(event);
}

void TextBox::enterEvent(QEvent* event) {
  update();
}

void TextBox::leaveEvent(QEvent* event) {
  update();
}

void TextBox::mousePressEvent(QMouseEvent* event) {
  if(is_placeholder_shown()) {
    m_line_edit->setFocus();
  }
  StyledWidget::mousePressEvent(event);
}

void TextBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    m_current = m_submission;
    m_line_edit->setText(m_current);
    m_current_signal(m_current);
  } else {
    StyledWidget::keyPressEvent(event);
  }
}

void TextBox::paintEvent(QPaintEvent* event) {
  auto computed_style = compute_style();
  auto placeholder_style = QString(
    R"(QLabel {
      background-color: rgba(0,0,0,0%);
      border-color: red;)");
  auto line_edit_style = QString("QLineEdit {");
  line_edit_style += "border-style: solid;";
  for(auto& property : computed_style.get_properties()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        line_edit_style += "background-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderTopSize& size) {
        auto property = "border-top-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const BorderRightSize& size) {
        auto property = "border-right-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const BorderBottomSize& size) {
        auto property = "border-bottom-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const BorderLeftSize& size) {
        auto property = "border-left-width: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const BorderTopColor& color) {
        line_edit_style += "border-top-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderRightColor& color) {
        line_edit_style += "border-right-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderBottomColor& color) {
        line_edit_style += "border-bottom-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const BorderLeftColor& color) {
        line_edit_style += "border-left-color: " +
          color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
      },
      [&] (const TextColor& color) {
        auto property = "color: " +
            color.get_expression().as<QColor>().name(QColor::HexArgb) + ";";
        if(is_placeholder_shown()) {
          placeholder_style += property;
        } else {
          line_edit_style += property;
        }
      },
      [&] (const TextAlign& alignment) {
        if(is_placeholder_shown()) {
          m_placeholder->setAlignment(
            alignment.get_expression().as<Qt::Alignment>());
        } else {
          m_line_edit->setAlignment(
            alignment.get_expression().as<Qt::Alignment>());
        }
      },
      [&] (const Font& font) {
        if(is_placeholder_shown()) {
          m_placeholder->setFont(font.get_expression().as<QFont>());
        } else {
          m_line_edit->setFont(font.get_expression().as<QFont>());
        }
      },
      [&] (const PaddingTop& size) {
        auto property = "padding-top: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const PaddingRight& size) {
        auto property = "padding-right: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const PaddingBottom& size) {
        auto property = "padding-bottom: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      },
      [&] (const PaddingLeft& size) {
        auto property = "padding-left: " + QString::number(
          size.get_expression().as<int>()) + "px;";
        line_edit_style += property;
        placeholder_style += property;
      });
  }
  line_edit_style += "}";
  if(line_edit_style != m_line_edit->styleSheet()) {
    m_line_edit->setStyleSheet(line_edit_style);
  }
  placeholder_style += "}";
  if(placeholder_style != m_placeholder->styleSheet()) {
    m_placeholder->setStyleSheet(placeholder_style);
  }
  StyledWidget::paintEvent(event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  m_placeholder->resize(event->size());
  update_display_text();
  update_placeholder_text();
  StyledWidget::resizeEvent(event);
}

void TextBox::on_editing_finished() {
  if(!is_read_only()) {
    m_submission = m_current;
    m_submit_signal(m_submission);
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_current = text;
  m_current_signal(m_current);
}

void TextBox::on_text_changed(const QString& text) {
  update_placeholder_text();
}

bool TextBox::is_placeholder_shown() const {
  return !is_read_only() && m_current.isEmpty() &&
    !m_placeholder_text.isEmpty();
}

QString TextBox::get_elided_text(const QFontMetrics& font_metrics,
    const QString& text) const {
  auto option = QStyleOptionFrame();
  option.initFrom(m_line_edit);
  option.rect = m_line_edit->contentsRect();
  option.lineWidth = 0;
  option.midLineWidth = 0;
  option.state |= QStyle::State_Sunken;
  if(is_read_only()) {
    option.state |= QStyle::State_ReadOnly;
  }
  option.features = QStyleOptionFrame::None;
  auto rect = m_line_edit->style()->subElementRect(QStyle::SE_LineEditContents,
    &option, m_line_edit);
  return font_metrics.elidedText(text, Qt::ElideRight, rect.width());
}

void TextBox::elide_text() {
  m_line_edit->setText(get_elided_text(m_line_edit->fontMetrics(), m_current));
  m_line_edit->setCursorPosition(0);
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else if(m_line_edit->text() != m_current) {
    m_line_edit->setText(m_current);
  }
}

void TextBox::update_placeholder_text() {
  if(is_placeholder_shown()) {
    m_placeholder->setText(get_elided_text(m_placeholder->fontMetrics(),
      m_placeholder_text));
    m_placeholder->show();
  } else {
    m_placeholder->hide();
  }
}
