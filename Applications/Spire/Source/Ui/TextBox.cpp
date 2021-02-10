#include "Spire/Ui/TextBox.hpp"
#include <array>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BACKGROUND_COLOR = QColor("#FFFFFF");
  const auto BACKGROUND_WARNING_COLOR = QColor("#FFF1F1");
  const auto BORDER_COLOR = QColor("#C8C8C8");
  const auto BORDER_WARNING_COLOR = QColor("#B71C1C");
  const auto WARNING_FADE_OUT_TIME_LINE_FRAME = 10;
  const auto WARNING_FADE_OUT_TIME_MS = 300;
  const auto WARNING_SHOW_DELAY_MS = 250;
  const auto WARNING_PROPERTY_NAME = "warning";

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
  };

  auto get_color_step(const QColor& start_color, const QColor& end_color) {
    return std::array<int, 3>{get_fade_out_step(start_color.red(), end_color.red()),
      get_fade_out_step(start_color.green(), end_color.green()),
      get_fade_out_step(start_color.blue(), end_color.blue())};
  }

  const auto BACKGROUND_COLOR_STEP = get_color_step(BACKGROUND_WARNING_COLOR,
    BACKGROUND_COLOR);
  const auto BORDER_COLOR_STEP = get_color_step(BORDER_WARNING_COLOR,
    BORDER_COLOR);

  auto get_fade_out_color(const QColor& color, const std::array<int, 3>& steps,
      int frame) {
    return QColor(color.red() + steps[0] * frame,
      color.green() + steps[1] * frame, color.blue() + steps[2] * frame);
  };
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
  setFont(font);
  m_style_sheet = QString(R"(
    #TextBox {
      background-color: %1;
      border: %2px solid %3;
      color: #000000;
      padding-left: %5px;
      padding-right: %5px;
    }
    #TextBox:hover {
      border-color: #4B23A0;
    }
    #TextBox:focus {
      border-color: #4B23A0;
    }
    #TextBox:read-only {
      background-color: #00000000;
      border: none;
      padding-left: 0px;
      padding-right: 0px;
    }
    #TextBox:disabled {
      background-color: #F5F5F5;
      border-color: #C8C8C8;
      color: #C8C8C8;
    }
    #TextBox:read-only:disabled {
      background-color: #00000000;
      border: none;
    })").
      arg(BACKGROUND_COLOR.name(QColor::HexArgb)).arg(scale_width(1)).
      arg(BORDER_COLOR.name(QColor::HexArgb)).arg(scale_width(8));
  setStyleSheet(m_style_sheet);
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

void TextBox::play_warning() {
  if(!isEnabled() || isReadOnly() || property(WARNING_PROPERTY_NAME).toBool()) {
    return;
  }
  setStyleSheet(m_style_sheet +
    get_warning_style_sheet(BACKGROUND_WARNING_COLOR, BORDER_WARNING_COLOR));
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
  auto background_color = get_fade_out_color(BACKGROUND_WARNING_COLOR,
    BACKGROUND_COLOR_STEP, frame);
  auto border_color = get_fade_out_color(BORDER_WARNING_COLOR,
    BORDER_COLOR_STEP, frame);
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
  setCursorPosition(0);
}
