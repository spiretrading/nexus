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
using namespace Spire;

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }
}

TitleBar::TitleBar(QWidget* body, QWidget* parent)
    : TitleBar(QImage(), body, parent) {}

TitleBar::TitleBar(const QImage& icon, QWidget* body, QWidget* parent)
    : TitleBar(icon, icon, body, parent) {}

TitleBar::TitleBar(const QImage& icon, const QImage& unfocused_icon,
    QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_is_dragging(false),
      m_body(body) {
  setFixedHeight(scale_height(26));
  setStyleSheet("background-color: #F5F5F5;");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  if(icon.isNull()) {
    m_default_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    m_icon = new IconButton(m_default_icon, this);
  } else if(unfocused_icon.isNull()) {
    m_default_icon = icon.scaled(ICON_SIZE());
    m_icon = new IconButton(m_default_icon, this);
  } else {
    m_default_icon = icon.scaled(ICON_SIZE());
    m_unfocused_icon = unfocused_icon.scaled(ICON_SIZE());
    m_icon = new IconButton(m_default_icon, m_unfocused_icon, this);
  }
  m_icon->setFocusPolicy(Qt::FocusPolicy::NoFocus);
  m_icon->setEnabled(false);
  layout->addWidget(m_icon);
  m_title_label = new QLabel("", this);
  m_title_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
  set_title_text_stylesheet(QColor("#000000"));
  layout->addWidget(m_title_label);
  layout->addSpacerItem(new QSpacerItem(scale_width(8), 10, QSizePolicy::Fixed,
    QSizePolicy::Expanding));
  auto button_size = scale(32, 26);
  auto minimize_box = QRect(translate(11, 12), scale(10, 2));
  m_minimize_button = new IconButton(
    imageFromSvg(":/icons/minimize-black.svg", button_size, minimize_box),
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
  m_maximize_button = new IconButton(
    imageFromSvg(":/icons/maximize-black.svg", button_size, maximize_box),
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
  m_restore_button = new IconButton(
    imageFromSvg(":/icons/unmaximize-black.svg",
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
  m_close_button = new IconButton(
    imageFromSvg(":/icons/close-black.svg", button_size, close_box),
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
  connect(window(), &QWidget::windowTitleChanged,
    [=] (auto& title) {on_window_title_change(title);});
  window()->installEventFilter(this);
  //qApp->installNativeEventFilter(this);
}

void TitleBar::set_icon(const QImage& icon) {
  if(icon.isNull()) {
    m_default_icon = QImage(ICON_SIZE(), QImage::Format::Format_ARGB32);
    set_icon(m_default_icon);
    return;
  }
  m_default_icon = icon.scaled(ICON_SIZE());
  m_icon->set_icon(m_default_icon);
}

void TitleBar::set_icon(const QImage& icon, const QImage& unfocused_icon) {
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

QLabel* TitleBar::get_title_label() const {
  return m_title_label;
}

//#ifdef Q_OS_WIN
//bool TitleBar::nativeEventFilter(const QByteArray& event_type, void* message,
//    long* result) {
//  auto msg = static_cast<MSG*>(message);
//  if(msg->message == WM_SYSCOMMAND &&
//      reinterpret_cast<HWND>(window()->winId()) == msg->hwnd) {
//    if(msg->wParam == SC_MAXIMIZE) {
//      on_maximize_button_press();
//      return true;
//    } else if(msg->wParam == SC_RESTORE && !window()->isMinimized()) {
//      on_restore_button_press();
//      return true;
//    }
//  }
//  return false;
//}
//#else
//bool TitleBar::nativeEventFilter(const QByteArray& event_type, void* message,
//    long* result) {
//  return false;
//}
//#endif

bool TitleBar::eventFilter(QObject* watched, QEvent* event) {
  if(watched == window()) {
    if(event->type() == QEvent::WindowDeactivate) {
      set_title_text_stylesheet(QColor("#A0A0A0"));
      m_icon->set_icon(m_unfocused_icon);
    } else if(event->type() == QEvent::WindowActivate) {
      set_title_text_stylesheet(QColor("#000000"));
      m_icon->set_icon(m_default_icon);
    } else if(event->type() == QEvent::WindowStateChange) {
      if(window()->isMaximized()) {
        m_maximize_button->hide();
        m_restore_button->show();
      } else {
        m_maximize_button->show();
        m_restore_button->hide();
      }
    }
  }
  return QWidget::eventFilter(watched, event);
}

// TODO: test that this works as expected for windows with a fixed maximum size
//void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
//  if(window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint)) {
//    if(window()->isMaximized()) {
//      on_restore_button_press();
//    } else {
//      on_maximize_button_press();
//    }
//  }
//}

void TitleBar::resizeEvent(QResizeEvent* event) {
  on_window_title_change(window()->windowTitle());
}

void TitleBar::on_window_title_change(const QString& title) {
  QFontMetrics metrics(m_title_label->font());
  auto shortened_text = metrics.elidedText(title,
    Qt::ElideRight, m_title_label->width());
  m_title_label->setText(shortened_text);
}

void TitleBar::on_minimize_button_press() {
  window()->showMinimized();
}

void TitleBar::on_maximize_button_press() {
  window()->showMaximized();
}

void TitleBar::on_restore_button_press() {
  window()->showNormal();
}

void TitleBar::on_close_button_press() {
  window()->close();
}

void TitleBar::set_title_text_stylesheet(const QColor& font_color) {
  m_title_label->setStyleSheet(QString(
    R"(color: %2;
       font-family: Roboto;
       font-size: %1px;)").arg(scale_height(12)).arg(font_color.name()));
}
