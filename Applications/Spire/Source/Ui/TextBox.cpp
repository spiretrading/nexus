#include "Spire/Ui/TextBox.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto WARNING_FADE_OUT_TIME_LINE_FRAME = 10;
  const auto WARNING_FADE_OUT_TIME_MS = 300;
  const auto WARNING_SHOW_DELAY_MS = 250;
  const auto WARNING_PROPERTY_NAME = "warning";

  auto create_style_sheet(const QString& state, const TextBox::Style& style) {
    auto default_sheet = QString(R"(
      #TextBox%1 {
        background-color: %2;
        border-color: %3;
        margin: 0px;
        outline: none;
      )").arg(state).arg(style.m_background_color.name(QColor::HexArgb)).
        arg(style.m_border_color.name(QColor::HexArgb));
    auto border_sheet = QString();
    if(style.m_borders) {
      border_sheet = QString(R"(
        border-bottom-width: %1px;
        border-left-width: %2px;
        border-right-width: %3px;
        border-style: solid;
        border-top-width: %4px;
      )").arg(style.m_borders->bottom()).arg(style.m_borders->left()).
        arg(style.m_borders->right()).arg(style.m_borders->top());
    }
    auto padding_sheet = QString();
    if(style.m_paddings) {
      padding_sheet = QString(R"(
        padding-bottom: %1px;
        padding-left: %2px;
        padding-right: %3px;
        padding-top: %4px;
      )").arg(style.m_paddings->bottom()).arg(style.m_paddings->left()).
        arg(style.m_paddings->right()).arg(style.m_paddings->top());
    }
    auto text_color_sheet = QString();
    if(style.m_text_color) {
      text_color_sheet = QString(R"(
        color: %1;
      )").arg(style.m_text_color->name(QColor::HexArgb));
    }
    auto font_sheet = QString();
    if(style.m_font) {
      auto weight = [&style] () -> QString {
        switch(style.m_font->weight()) {
        case QFont::Normal:
          return "normal";
        case QFont::Medium:
          return "500";
        case QFont::Bold:
          return "bold";
        case QFont::Black:
          return "900";
        default:
          return QString::number(100 + style.m_font->weight() * 8);
        }
      }();
      font_sheet = QString(R"(
        font-family: "%1";
        font-size: %2px;
        font-weight: %3;
      )").arg(style.m_font->family()).arg(style.m_font->pixelSize()).arg(weight);
    }
    auto closing_bracket = QString("}");
    auto style_sheet = QString();
    style_sheet.reserve(default_sheet.length() + border_sheet.length() +
      padding_sheet.length() + text_color_sheet.length() + font_sheet.length() +
      closing_bracket.length());
    return style_sheet.append(default_sheet).append(border_sheet).
      append(padding_sheet).append(text_color_sheet).append(font_sheet).
      append(closing_bracket);
  }

  auto create_text_box_style_sheet(const TextBox::Styles& styles) {
    auto default_style_sheet = create_style_sheet("", styles.m_style);
    auto hover_style_sheet = create_style_sheet(":hover", styles.m_hover_style);
    auto focus_style_sheet = create_style_sheet(":focus", styles.m_focus_style);
    auto disabled_style_sheet = create_style_sheet(":disabled",
      styles.m_disabled_style);
    auto read_only_style_sheet = create_style_sheet(":read-only",
      styles.m_read_only_style);
    auto read_only_disabled_style_sheet = create_style_sheet(
      ":read-only:disabled", styles.m_read_only_style);
    auto style_sheet = QString();
    style_sheet.reserve(default_style_sheet.length() +
      hover_style_sheet.length() + focus_style_sheet.length() +
      disabled_style_sheet.length() + read_only_style_sheet.length() +
      read_only_disabled_style_sheet.length());
    return style_sheet.append(default_style_sheet).
      append(hover_style_sheet).append(focus_style_sheet).
      append(disabled_style_sheet).append(read_only_style_sheet).
      append(read_only_disabled_style_sheet);
  }

  auto get_warning_style_sheet(const QColor& background_color,
      const QColor& border_color) {
    return QString(R"(
      #TextBox[%1=true] {
        background-color: %2;
        border-color: %3;
      })").
        arg(WARNING_PROPERTY_NAME).arg(background_color.name(QColor::HexArgb)).
        arg(border_color.name(QColor::HexArgb));
  }

  auto get_fade_out_step(int start, int end) {
    return (end - start) / WARNING_FADE_OUT_TIME_LINE_FRAME;
  }

  auto get_color_step(const QColor& start_color, const QColor& end_color) {
    return std::array{get_fade_out_step(start_color.red(), end_color.red()),
      get_fade_out_step(start_color.green(), end_color.green()),
      get_fade_out_step(start_color.blue(), end_color.blue())};
  }

  auto get_fade_out_color(const QColor& color, const std::array<int, 3>& steps,
      int frame) {
    return QColor(color.red() + steps[0] * frame,
      color.green() + steps[1] * frame, color.blue() + steps[2] * frame);
  }
}

TextBox::TextBox(QWidget* parent)
  : TextBox("", parent) {}

TextBox::TextBox(const QString& text, QWidget* parent)
    : QLineEdit(text, parent),
      m_text(text),
      m_submitted_text(text) {
  setObjectName("TextBox");
  setFrame(false);
  auto font = QFont("Roboto");
  font.setPixelSize(scale_width(12));
  font.setWeight(QFont::Normal);
  m_styles.m_style.m_background_color = QColor("#FFFFFF");
  m_styles.m_style.m_border_color = QColor("#C8C8C8");
  m_styles.m_style.m_borders = {{scale_width(1), scale_height(1),
    scale_width(1), scale_height(1)}};
  m_styles.m_style.m_paddings = {{scale_width(8), 0, scale_width(8), 0}};
  m_styles.m_style.m_font = font;
  m_styles.m_style.m_text_color = QColor("#000000");
  m_styles.m_hover_style.m_background_color =
    m_styles.m_style.m_background_color;
  m_styles.m_hover_style.m_border_color = QColor("#4B23A0");
  m_styles.m_focus_style.m_background_color =
    m_styles.m_style.m_background_color;
  m_styles.m_focus_style.m_border_color = QColor("#4B23A0");
  m_styles.m_read_only_style.m_background_color = QColor("#00000000");
  m_styles.m_read_only_style.m_borders = {{0, 0, 0, 0}};
  m_styles.m_read_only_style.m_paddings = {{0, 0, 0, 0}};
  m_styles.m_disabled_style.m_background_color = QColor("#F5F5F5");
  m_styles.m_disabled_style.m_border_color = QColor("#C8C8C8");
  m_styles.m_disabled_style.m_text_color = QColor("#C8C8C8");
  m_styles.m_warning_style.m_background_color = QColor("#FFF1F1");
  m_styles.m_warning_style.m_border_color = QColor("#B71C1C");
  set_styles(m_styles);
  m_warning_time_line.setDuration(WARNING_FADE_OUT_TIME_MS);
  m_warning_time_line.setFrameRange(0, WARNING_FADE_OUT_TIME_LINE_FRAME);
  m_warning_time_line.setEasingCurve(QEasingCurve::Linear);
  connect(&m_warning_time_line, &QTimeLine::frameChanged, this,
    &TextBox::on_warning_fade_out);
  connect(&m_warning_time_line, &QTimeLine::finished, this,
    &TextBox::on_warning_finished);
  connect(this, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(this, &QLineEdit::selectionChanged, this,
    &TextBox::on_selection_changed);
  connect(this, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
}

const QString& TextBox::get_text() const {
  return m_text;
}

void TextBox::set_text(const QString& text) {
  m_text = text;
  m_submitted_text = text;
  if(!isEnabled() || isReadOnly() || !hasFocus()) {
    elide_text();
  } else {
    QLineEdit::setText(m_text);
  }
}

const TextBox::Styles& TextBox::get_styles() const {
  return m_styles;
}

void TextBox::set_styles(const Styles& styles) {
  m_styles = styles;
  m_style_sheet = create_text_box_style_sheet(m_styles);
  setStyleSheet(m_style_sheet);
  m_warning_background_color_step = get_color_step(
    m_styles.m_warning_style.m_background_color,
    m_styles.m_style.m_background_color);
  m_warning_border_color_step = get_color_step(
    m_styles.m_warning_style.m_border_color, m_styles.m_style.m_border_color);
}

void TextBox::play_warning() {
  if(!isEnabled() || isReadOnly() || property(WARNING_PROPERTY_NAME).toBool()) {
    return;
  }
  setStyleSheet(m_style_sheet +
    get_warning_style_sheet(m_styles.m_warning_style.m_background_color,
    m_styles.m_warning_style.m_border_color));
  setProperty(WARNING_PROPERTY_NAME, true);
  style()->unpolish(this);
  style()->polish(this);
  QTimer::singleShot(WARNING_SHOW_DELAY_MS, this, &TextBox::on_warning_timeout);
}

connection TextBox::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection TextBox::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void TextBox::changeEvent(QEvent* event) {
  switch(event->type()) {
    case QEvent::ReadOnlyChange:
    case QEvent::FontChange:
    case QEvent::StyleChange:
      elide_text();
      break;
  }
  QLineEdit::changeEvent(event);
}

void TextBox::focusInEvent(QFocusEvent* event) {
  if(!isReadOnly()) {
    auto reason = event->reason();
    if(reason != Qt::ActiveWindowFocusReason &&
      reason != Qt::PopupFocusReason) {
      QLineEdit::setText(m_text);
    }
  }
  QLineEdit::focusInEvent(event);
}

void TextBox::focusOutEvent(QFocusEvent* event) {
  auto reason = event->reason();
  if(reason != Qt::ActiveWindowFocusReason && reason != Qt::PopupFocusReason) {
    elide_text();
  }
  QLineEdit::focusOutEvent(event);
}

void TextBox::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Escape) {
    m_text = m_submitted_text;
    QLineEdit::setText(m_text);
    m_current_signal(m_text);
  }
  QLineEdit::keyPressEvent(event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  elide_text();
  QLineEdit::resizeEvent(event);
}

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

void TextBox::on_editing_finished() {
  if(!isReadOnly()) {
    m_submitted_text = m_text;
    m_submit_signal(m_submitted_text);
  }
}

void TextBox::on_selection_changed() {
  if(isReadOnly()) {
    auto text = selectedText();
    if(text.endsWith(QChar(0x2026)) || text.endsWith(QStringLiteral("..."))) {
      QLineEdit::setText(m_text);
      setSelection(0, m_text.length());
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_text = text;
  m_current_signal(m_text);
}

void TextBox::on_warning_timeout() {
  m_warning_time_line.start();
}

void TextBox::on_warning_fade_out(int frame) {
  auto background_color = get_fade_out_color(
    m_styles.m_warning_style.m_background_color,
    m_warning_background_color_step, frame);
  auto border_color = get_fade_out_color(m_styles.m_warning_style.m_border_color,
    m_warning_border_color_step, frame);
  setStyleSheet(m_style_sheet +
    get_warning_style_sheet(background_color, border_color));
}

void TextBox::on_warning_finished() {
  setProperty(WARNING_PROPERTY_NAME, false);
  style()->unpolish(this);
  style()->polish(this);
}

void TextBox::elide_text() {
  auto font_metrics = fontMetrics();
  auto panel = QStyleOptionFrame();
  initStyleOption(&panel);
  auto rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
  QLineEdit::setText(font_metrics.elidedText(m_text, Qt::ElideRight,
    rect.width()));
}
