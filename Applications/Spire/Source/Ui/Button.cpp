#include "Spire/Ui/Button.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

Button::Button(TextBox* text_box, QWidget* parent)
    : Box(parent),
      m_text_box(text_box) {
  m_text_box->setParent(this);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_text_box);
  setLayout(layout);
  m_text_box->installEventFilter(this);
  connect(m_text_box, &TextBox::selectionChanged, [=] {
    m_text_box->deselect();
  });
}

TextBox* Button::get_text_box() const {
  return m_text_box;
}

void Button::set_text(const QString& label) {
  m_text_box->set_text(label);
}

connection Button::connect_clicked_signal(
    const ClickedSignal::slot_type& slot) const {
  return m_clicked_signal.connect(slot);
}

bool Button::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_text_box) {
    switch(event->type()) {
    case QEvent::MouseButtonRelease:
      event->ignore();
      break;
    }
  } 
  return Box::eventFilter(watched, event);
}

void Button::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
      if(!event->isAutoRepeat()) {
        m_clicked_signal();
      }
      break;
    default:
      Box::keyPressEvent(event);
  }
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
    m_clicked_signal();
  }
  Box::mouseReleaseEvent(event);
}

Button* Spire::make_button(const QString& label, QWidget* parent) {
  auto text_box = new TextBox(label);
  text_box->setReadOnly(true);
  text_box->setAlignment(Qt::AlignCenter);
  text_box->setFocusPolicy(Qt::NoFocus);
  text_box->setContextMenuPolicy(Qt::NoContextMenu);
  text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto button = new Button(text_box, parent);
  button->setFocusPolicy(Qt::StrongFocus);
  auto text_styles = text_box->get_styles();
  text_styles.m_style.m_text_color = QColor("#000000");
  text_styles.m_hover_style.m_text_color = QColor("#FFFFFF");
  text_styles.m_disabled_style.m_text_color = QColor("#C8C8C8");
  text_box->set_styles(text_styles);
  auto style = button->get_style();
  style.m_background_color = QColor("#EBEBEB");
  style.m_borders = {{0, 0, 0, 0}};
  style.m_size = QSize(scale(180, 26));
  button->set_style(style);
  auto hover_style = button->get_hover_style();
  hover_style.m_background_color = QColor("#4B23A0");
  hover_style.m_borders = {{0, 0, 0, 0}};
  button->set_hover_style(hover_style);
  auto focus_style = button->get_focus_style();
  focus_style.m_background_color = QColor("#EBEBEB");
  focus_style.m_border_color = QColor("#4B23A0");
  hover_style.m_borders = {{scale_width(1), scale_height(1), scale_width(1),
    scale_height(1)}};
  button->set_focus_style(focus_style);
  auto disabled_style = button->get_disabled_style();
  disabled_style.m_background_color = QColor("#EBEBEB");
  disabled_style.m_borders = {{0, 0, 0, 0}};
  button->set_disabled_style(disabled_style);
  return button;
}
