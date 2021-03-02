#include "Spire/Ui/TextBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto create_default_text_box_style_sheet() {
    return QString(R"(
      QLineEdit {
        background-color: #FFFFFF;
        border: %1px solid #C8C8C8;
        color: #000000;
        font-family: "Roboto";
        font-size: %2px;
        font-weight: normal;
        padding-left: %3px;
        padding-right: %3px;
      }
      QLineEdit:hover {
        border-color: #4B23A0;
      }
      QLineEdit:focus {
        border-color: #4B23A0;
      }
      QLineEdit:read-only {
        background-color: #00000000;
        border: none;
        padding-left: 0px;
        padding-right: 0px;
      }
      QLineEdit:disabled {
        background-color: #F5F5F5;
        border-color: #C8C8C8;
        color: #C8C8C8;
      }
      QLineEdit:read-only:disabled {
        background-color: #00000000;
        border: none;
      })").arg(scale_width(1)).arg(scale_width(12)).arg(scale_width(8));
  }
}

TextBox::TextBox(QWidget* parent)
  : TextBox("", parent) {}

TextBox::TextBox(const QString& current, QWidget* parent)
    : QWidget(parent),
      m_current(current) {
  m_line_edit = new QLineEdit(m_current, this);
  m_line_edit->setFrame(false);
  m_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_line_edit);
  m_line_edit->installEventFilter(this);
  setStyleSheet(create_default_text_box_style_sheet());
  setFocusProxy(m_line_edit);
  connect(m_line_edit, &QLineEdit::editingFinished, this,
    &TextBox::on_editing_finished);
  connect(m_line_edit, &QLineEdit::selectionChanged, this,
    &TextBox::on_selection_changed);
  connect(m_line_edit, &QLineEdit::textEdited, this, &TextBox::on_text_edited);
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

void TextBox::set_read_only(bool read_only) {
  m_line_edit->setReadOnly(read_only);
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

bool TextBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_line_edit) {
    switch(event->type()) {
    case QEvent::StyleChange:
      if(m_old_style_sheet != styleSheet()) {
        m_old_style_sheet = styleSheet();
        update_display_text();
      }
      break;
    case QEvent::ReadOnlyChange:
    case QEvent::EnabledChange:
      update_display_text();
      break;
    case QEvent::FocusIn:
      if(!is_read_only()) {
        auto reason = static_cast<QFocusEvent*>(event)->reason();
        if(reason != Qt::ActiveWindowFocusReason &&
            reason != Qt::PopupFocusReason) {
          update_display_text();
        }
      }
      break;
    case QEvent::FocusOut:
      {
        auto reason = static_cast<QFocusEvent*>(event)->reason();
        if(reason != Qt::ActiveWindowFocusReason &&
            reason != Qt::PopupFocusReason) {
          update_display_text();
        }
      }
      break;
    case QEvent::KeyPress:
      if(!is_read_only() && static_cast<QKeyEvent*>(event)->key() ==
          Qt::Key_Escape) {
        m_current = m_submission;
        m_line_edit->setText(m_current);
        m_current_signal(m_current);
      }
      break;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TextBox::resizeEvent(QResizeEvent* event) {
  update_display_text();
  QWidget::resizeEvent(event);
}

QSize TextBox::sizeHint() const {
  return scale(160, 30);
}

void TextBox::on_editing_finished() {
  if(!is_read_only()) {
    m_submission = m_current;
    m_submit_signal(m_submission);
  }
}

void TextBox::on_selection_changed() {
  if(is_read_only()) {
    auto text = m_line_edit->selectedText();
    if(text.endsWith(QChar(0x2026)) || text.endsWith(QStringLiteral("..."))) {
      m_line_edit->setText(m_current);
      m_line_edit->setSelection(0, m_current.length());
    }
  }
}

void TextBox::on_text_edited(const QString& text) {
  m_current = text;
  m_current_signal(m_current);
}

void TextBox::elide_text() {
  auto font_metrics = m_line_edit->fontMetrics();
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
  m_line_edit->setText(font_metrics.elidedText(m_current, Qt::ElideRight,
    rect.width()));
  m_line_edit->setCursorPosition(0);
}

void TextBox::update_display_text() {
  if(!isEnabled() || is_read_only() || !hasFocus()) {
    elide_text();
  } else {
    if(m_line_edit->text() != m_current) {
      m_line_edit->setText(m_current);
    }
  }
}
