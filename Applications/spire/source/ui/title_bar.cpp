#include "spire/ui/title_bar.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/window.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace spire;

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }
}

title_bar::title_bar(QWidget* body, QWidget* parent)
    : title_bar(QImage(), body, parent) {}

title_bar::title_bar(const QImage& icon, QWidget* body, QWidget* parent)
    : title_bar(icon, icon, body, parent) {}

title_bar::title_bar(const QImage& icon, const QImage& unfocused_icon,
    QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_is_dragging(false),
      m_window_maximized(false) {
  setFixedHeight(scale_height(26));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  if(icon.isNull()) {
    m_default_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    m_icon = new icon_button(m_default_icon, this);
  } else if(unfocused_icon.isNull()) {
    m_default_icon = icon.scaled(ICON_SIZE());
    m_icon = new icon_button(m_default_icon, this);
  } else {
    m_default_icon = icon.scaled(ICON_SIZE());
    m_unfocused_icon = unfocused_icon.scaled(ICON_SIZE());
    m_icon = new icon_button(m_default_icon, m_unfocused_icon, this);
  }
  m_icon->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_icon->setEnabled(false);
  layout->addWidget(m_icon);
  m_title_label = new QLabel("", this);
  m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  set_title_text_stylesheet(QColor("#000000"));
  layout->addWidget(m_title_label);
  layout->addSpacerItem(new QSpacerItem(scale_width(8), 10, QSizePolicy::Fixed,
    QSizePolicy::Expanding));
  auto button_size = scale(32, 26);
  auto minimize_box = QRect(translate(11, 12), scale(10, 2));
  m_minimize_button = new icon_button(
    imageFromSvg(":/icons/minimize-darker-grey.svg", button_size, minimize_box),
    imageFromSvg(":/icons/minimize-black.svg", button_size, minimize_box),
    imageFromSvg(":/icons/minimize-grey.svg", button_size, minimize_box), this);
  m_minimize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_minimize_button->set_default_style("background-color: #F5F5F5;");
  m_minimize_button->set_hover_style("background-color: #EBEBEB;");
  m_minimize_button->setVisible(
    window()->windowFlags().testFlag(Qt::WindowMinimizeButtonHint));
  m_minimize_button->connect_clicked_signal(
    [=] { on_minimize_button_press(); });
  layout->addWidget(m_minimize_button);
  auto maximize_box = QRect(translate(11, 8), scale(10, 10));
  m_maximize_button = new icon_button(
    imageFromSvg(":/icons/maximize-darker-grey.svg", button_size, maximize_box),
    imageFromSvg(":/icons/maximize-black.svg", button_size, maximize_box),
    imageFromSvg(":/icons/maximize-grey.svg", button_size, maximize_box), this);
  m_maximize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_maximize_button->set_default_style("background-color: #F5F5F5;");
  m_maximize_button->set_hover_style("background-color: #EBEBEB;");
  m_maximize_button->setVisible(
    window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint));
  m_maximize_button->connect_clicked_signal(
    [=] { on_maximize_button_press(); });
  layout->addWidget(m_maximize_button);
  auto restore_box = QRect(translate(11, 8), scale(10, 10));
  m_restore_button = new icon_button(
    imageFromSvg(":/icons/unmaximize-darker-grey.svg",
      button_size, restore_box),
    imageFromSvg(":/icons/unmaximize-black.svg", button_size, restore_box),
    imageFromSvg(":/icons/unmaximize-grey.svg", button_size, restore_box),
    this);
  m_restore_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_restore_button->set_default_style("background-color: #F5F5F5;");
  m_restore_button->set_hover_style("background-color: #EBEBEB;");
  m_restore_button->connect_clicked_signal([=] { on_restore_button_press(); });
  m_restore_button->hide();
  layout->addWidget(m_restore_button);
  auto close_box = QRect(translate(11, 8), scale(10, 10));
  m_close_button = new icon_button(
    imageFromSvg(":/icons/close-darker-grey.svg", button_size, close_box),
    imageFromSvg(":/icons/close-red.svg", button_size, close_box),
    imageFromSvg(":/icons/close-grey.svg", button_size, close_box),
    this);
  m_close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_close_button->set_default_style("background-color: #F5F5F5;");
  m_close_button->set_hover_style("background-color: #EBEBEB;");
  m_close_button->setVisible(
    window()->windowFlags().testFlag(Qt::WindowCloseButtonHint));
  m_close_button->connect_clicked_signal([=] { on_close_button_press(); });
  layout->addWidget(m_close_button);

  // TODO: GCC workaround
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto&& title) { this->on_window_title_change(title); });
  window()->installEventFilter(this);
#ifdef Q_OS_WIN
  qApp->installNativeEventFilter(this);
#endif
}

void title_bar::set_icon(const QImage& icon) {
  if(icon.isNull()) {
    m_default_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    set_icon(m_default_icon);
    return;
  }
  m_default_icon = icon.scaled(ICON_SIZE());
  m_icon->set_icon(m_default_icon);
}

void title_bar::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  if(icon.isNull()) {
    m_default_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    m_unfocused_icon = unfocused_icon;
    set_icon(m_default_icon, m_unfocused_icon);
    return;
  }
  if(unfocused_icon.isNull()) {
    m_default_icon = icon;
    m_unfocused_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    set_icon(m_default_icon, m_unfocused_icon);
    return;
  }
  m_default_icon = icon.scaled(ICON_SIZE());
  m_unfocused_icon = unfocused_icon.scaled(ICON_SIZE());
  m_icon->set_icon(m_default_icon, m_unfocused_icon);
}

bool title_bar::nativeEventFilter(const QByteArray& event_type, void* message,
    long* result) {
#ifdef Q_OS_WIN
  auto msg = static_cast<MSG*>(message);
  if(msg->message == WM_SYSCOMMAND &&
      reinterpret_cast<HWND>(window()->winId()) == msg->hwnd) {
    if(msg->wParam == SC_MAXIMIZE) {
      on_maximize_button_press();
      return true;
    } else if(msg->wParam == SC_RESTORE && !window()->windowState().testFlag(
        Qt::WindowMinimized)) {
      on_restore_button_press();
      return true;
    }
  }
#endif
  return false;
}

bool title_bar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowDeactivate) {
      set_title_text_stylesheet(QColor("#A0A0A0"));
      m_icon->set_icon(m_unfocused_icon);
    } else if(event->type() == QEvent::WindowActivate) {
      set_title_text_stylesheet(QColor("#000000"));
      m_icon->set_icon(m_default_icon);
    }
  }
  return QWidget::eventFilter(watched, event);
}

void title_bar::mouseDoubleClickEvent(QMouseEvent* event) {
  if(m_window_maximized) {
    on_restore_button_press();
  } else {
    on_maximize_button_press();
  }
}

void title_bar::mouseMoveEvent(QMouseEvent* event) {
  if(!m_is_dragging) {
    return;
  }
  if(m_window_maximized) {
    on_restore_button_press();
    window()->setGeometry(m_window_restore_geometry);
    auto mouse_screen_pos = QApplication::desktop()->screenGeometry(
      event->globalPos());
    auto mouse_screen_x = event->globalPos().x() - mouse_screen_pos.left();
    auto new_pos = QPoint(event->globalX() - (window()->width() / 2), 0);
    if(mouse_screen_x - width() < 0) {
      new_pos.setX(mouse_screen_pos.left());
    } else if(mouse_screen_x + width() > mouse_screen_pos.width()) {
      new_pos.setX(mouse_screen_pos.right() - width());
    }
    window()->move(new_pos);
  }
  auto delta = event->globalPos();
  delta -= m_last_mouse_pos;
  auto window_pos = window()->pos();
  window_pos += delta;
  m_last_mouse_pos = event->globalPos();
  window()->move(window_pos);
}

void title_bar::mousePressEvent(QMouseEvent* event)  {
  if(m_is_dragging || event->button() != Qt::LeftButton) {
    return;
  }
  m_is_dragging = true;
  m_last_mouse_pos = event->globalPos();
}

void title_bar::mouseReleaseEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) {
    return;
  }
  if(m_is_dragging) {
    m_window_restore_pos = window()->pos();
    m_is_dragging = false;
  }
}

void title_bar::resizeEvent(QResizeEvent* event) {
  on_window_title_change(window()->windowTitle());
}

void title_bar::on_window_title_change(const QString& title) {
  QFontMetrics metrics(m_title_label->font());
  auto shortened_text = metrics.elidedText(title,
    Qt::ElideRight, m_title_label->width());
  m_title_label->setText(shortened_text);
}

void title_bar::on_minimize_button_press() {
  if(m_minimize_button->isVisible()) {
    window()->showMinimized();
  }
}

void title_bar::on_maximize_button_press() {
  if(m_maximize_button->isVisible()) {
    m_window_maximized = true;
    m_maximize_button->setVisible(false);
    m_restore_button->setVisible(true);
    m_window_restore_geometry = window()->geometry();
    m_window_restore_pos = window()->pos();
    m_max_body_size = m_body->maximumSize();
    m_body->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    window()->setGeometry(
      QApplication::desktop()->availableGeometry(window()));
#ifdef Q_OS_WIN
    window()->setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    window()->show();
#endif
  }
}

void title_bar::on_restore_button_press() {
  if(m_restore_button->isVisible()) {
    m_window_maximized = false;
    m_maximize_button->setVisible(true);
    m_restore_button->setVisible(false);
    m_body->setMaximumSize(m_max_body_size);
    window()->setGeometry(m_window_restore_geometry);
    window()->move(m_window_restore_pos);
#ifdef Q_OS_WIN
    window()->setWindowFlag(Qt::WindowMaximizeButtonHint);
    window()->show();
#endif
  }
}

void title_bar::on_close_button_press() {
  if(m_close_button->isVisible()) {
    window()->close();
  }
}

void title_bar::set_title_text_stylesheet(const QColor& font_color) {
  m_title_label->setStyleSheet(QString(
    R"(color: %2;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)).arg(font_color.name()));
}
