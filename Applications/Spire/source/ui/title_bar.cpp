#include "spire/ui/title_bar.hpp"
#include <QHBoxLayout>
#include <QMouseEvent>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"




#include <QDebug>



using namespace boost;
using namespace boost::signals2;
using namespace spire;

title_bar::title_bar(QWidget* parent)
    : title_bar(QImage(), parent) {}

title_bar::title_bar(const QImage& icon, QWidget* parent)
    : title_bar(icon, icon, parent) {}

title_bar::title_bar(const QImage& icon, const QImage& unfocused_icon,
    QWidget* parent)
    : QWidget(parent),
      m_is_dragging(false) {
  setFixedHeight(scale_height(26));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  auto icon_size = QSize(scale_width(26), scale_height(26));
  m_icon = new icon_button(icon.scaled(icon_size),
    unfocused_icon.scaled(icon_size), this);
  m_icon->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_icon->setEnabled(false);
  layout->addWidget(m_icon);
  m_title_label = new QLabel("", this);
  m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_title_label->setStyleSheet(QString(
    R"(background-color: red; font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)));
  layout->addWidget(m_title_label);
  auto button_size = scale(32, 26);
  auto minimize_box = QRect(translate(11, 12), scale(10, 2));
  m_minimize_button = new icon_button(
    imageFromSvg(":/icons/minimize-grey.svg", button_size, minimize_box),
    imageFromSvg(":/icons/minimize-black.svg", button_size, minimize_box),
    this);
  m_minimize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_minimize_button->set_default_style("background-color: #F5F5F5;");
  m_minimize_button->set_hover_style("background-color: #EBEBEB;");
  m_minimize_button->connect_clicked_signal(
    [=] { on_minimize_button_press(); });
  layout->addWidget(m_minimize_button);
  auto maximize_box = QRect(translate(11, 8), scale(10, 10));
  m_maximize_button = new icon_button(
    imageFromSvg(":/icons/maximize-grey.svg", button_size, maximize_box),
    imageFromSvg(":/icons/maximize-black.svg", button_size, maximize_box),
    this);
  m_maximize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_maximize_button->set_default_style("background-color: #F5F5F5;");
  m_maximize_button->set_hover_style("background-color: #EBEBEB;");
  m_maximize_button->connect_clicked_signal(
    [=] { on_maximize_button_press(); });
  layout->addWidget(m_maximize_button);
  auto restore_box = QRect(translate(11, 8), scale(10, 10));
  m_restore_button = new icon_button(
    imageFromSvg(":/icons/unmaximize-grey.svg", button_size, restore_box),
    imageFromSvg(":/icons/unmaximize-black.svg", button_size, restore_box),
    this);
  m_restore_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_restore_button->setVisible(false);
  m_restore_button->set_default_style("background-color: #F5F5F5;");
  m_restore_button->set_hover_style("background-color: #EBEBEB;");
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  layout->addWidget(m_restore_button);
  auto close_box = QRect(translate(11, 8), scale(10, 10));
  m_close_button = new icon_button(
    imageFromSvg(":/icons/close-grey.svg", button_size, close_box),
    imageFromSvg(":/icons/close-red.svg", button_size, close_box), this);
  m_close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_close_button->set_default_style("background-color: #F5F5F5;");
  m_close_button->set_hover_style("background-color: #EBEBEB;");
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  layout->addWidget(m_close_button);
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto&& title) { on_window_title_change(title); });
}

void title_bar::set_icon(const QImage& icon) {
  m_icon->set_icon(icon.scaled(m_icon->get_icon().size()));
}

void title_bar::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_icon->set_icon(icon.scaled(m_icon->get_icon().size()),
    unfocused_icon.scaled(m_icon->get_icon().size()));
}

connection title_bar::connect_maximize_signal(
    const maximize_signal::slot_type& slot) const {
  return m_maximize_signal.connect(slot);
}

void title_bar::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ParentAboutToChange) {
    disconnect(window(), 0, this, 0);
  } else if(event->type() == QEvent::ParentChange) {
    connect(window(), &QWidget::windowTitleChanged,
      [=] (auto&& title) { on_window_title_change(title); });
  }
}

void title_bar::mouseDoubleClickEvent(QMouseEvent* event) {
  if(window()->windowState().testFlag(Qt::WindowMaximized)) {
    on_restore_button_press();
  } else {
    on_maximize_button_press();
  }
}

void title_bar::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  auto delta = event->globalPos();
  delta -= m_last_mouse_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_mouse_pos = event->globalPos();
  window()->move(window_pos);
}

void title_bar::mousePressEvent(QMouseEvent* event)  {
  if(m_is_dragging || event->button() != Qt::LeftButton ||
      window()->windowState().testFlag(Qt::WindowMaximized)) {
    return;
  }
  m_is_dragging = true;
  m_last_mouse_pos = event->globalPos();
}

void title_bar::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = false;
}

void title_bar::on_window_title_change(const QString& title) {
  m_title_label->setText(title);
  QFontMetrics metrics(m_title_label->font());
  qDebug() << m_title_label->width();
  auto shortened_text = metrics.elidedText(m_title_label->text(),
    Qt::ElideRight, m_title_label->width());
  m_title_label->setText(shortened_text);
}

void title_bar::on_minimize_button_press() {
  window()->showMinimized();
}

void title_bar::on_maximize_button_press() {
  m_maximize_signal();
  m_maximize_button->setVisible(false);
  m_restore_button->setVisible(true);
  window()->showMaximized();
}

void title_bar::on_restore_button_press() {
  m_maximize_button->setVisible(true);
  m_restore_button->setVisible(false);
  window()->showNormal();
}

void title_bar::on_close_button_press() {
  window()->close();
}
