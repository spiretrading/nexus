#include "Spire/Ui/TextBox.hpp"
#include <QEvent>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

TextBox::TextBox(QWidget* parent)
  : TextBox("", parent) {}

TextBox::TextBox(const QString& text, QWidget* parent)
    : QLineEdit(text, parent),
      m_text(text) {
  setObjectName("text_box");
  setFrame(false);
  setStyleSheet(QString(R"(
    #text_box {
      background-color: #FFFFFF;
      border: %1px solid #C8C8C8;
      color: #000000;
      font-family: "Roboto";
      font-size: %2px;
      padding-left: %3px;
      padding-right: %3px;
    }
    #text_box:hover {
      border-color: #4B23A0;
    }
    #text_box:focus {
      border-color: #4B23A0;
    }
    #text_box:read-only {
      background-color: #00000000;
      border: none;
      padding-left: 0px;
      padding-right: 0px;
    }
    #text_box:disabled {
      background-color: #F5F5F5;
      border-color: #C8C8C8;
      color: #C8C8C8;
    })")
    .arg(scale_width(1))
    .arg(scale_width(12))
    .arg(scale_width(8)));
  connect(this, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(this, &QLineEdit::selectionChanged, this,
    &TextBox::on_selection_changed);
  connect(this, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
}

QString TextBox::get_text() const {
  return m_text;
}

void TextBox::set_text(const QString& text) {
  m_text = text;
  if(!isEnabled() || isReadOnly() || !hasFocus()) {
    elide_text();
  } else {
    QLineEdit::setText(m_text);
  }
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
  if(event->type() == QEvent::ReadOnlyChange) {
    elide_text();
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
    m_submit_signal(m_text);
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

void TextBox::elide_text() {
  auto font_metrics = fontMetrics();
  auto panel = QStyleOptionFrame();
  initStyleOption(&panel);
  auto rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
  QLineEdit::setText(font_metrics.elidedText(m_text, Qt::ElideRight,
    rect.width()));
  setCursorPosition(0);
}
