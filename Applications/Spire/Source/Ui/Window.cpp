#include "Spire/Ui/Window.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QWindow>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include <windows.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TitleBar.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto RESIZE_AREA() {
    return scale(8, 8);
  }

  auto ICON_SIZE() {
    return scale(26, 26);
  }

  auto get_system_border_padding_size(int dpi) {
    return 4 * dpi / DPI;
  }

  auto get_system_frame_size(int dpi) {
    if(dpi < 144) {
      return 4;
    } else if(dpi < 240) {
      return 5;
    } else if(dpi < 336) {
      return 6;
    }
    return 7;
  }

  auto get_system_border_size(int x_dpi, int y_dpi) {
    return QSize(
      get_system_frame_size(x_dpi) + get_system_border_padding_size(x_dpi),
      get_system_frame_size(y_dpi) + get_system_border_padding_size(y_dpi));
  }

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26), QRect(translate(8, 8),
      scale(10, 10)));
  }
}

Window::Window(QWidget* parent)
    : QWidget(parent),
      m_body(nullptr),
      m_is_resizable(true) {
  setWindowFlags(windowFlags() | Qt::CustomizeWindowHint |
    Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
  m_title_bar = new TitleBar(make_svg_window_icon(":/Icons/spire.svg"), this);
  auto box_body = new QWidget();
  auto layout = make_vbox_layout(box_body);
  layout->setAlignment(Qt::AlignTop);
  layout->addWidget(m_title_bar);
  auto box = new Box(box_body);
  box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  proxy_style(*this, *box);
  update_style(*this, [] (auto& style) {
    style.get(Any()).
      set(BackgroundColor(QColor(0xF5F5F5))).
      set(border(scale_width(1), QColor(0xA0A0A0)));
    style.get(Highlighted()).
      set(border_color(QColor(0x7F5EEC)));
  });
  enclose(*this, *box);
  find_stylist(*this).connect_match_signal(Highlighted(),
    std::bind_front(&Window::on_highlighted, this));
}

void Window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void Window::set_svg_icon(const QString& icon_path) {
  set_icon(make_svg_window_icon(icon_path));
}

void Window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool Window::event(QEvent* event) {
  if(event->type() == QEvent::WinIdChange) {
    set_window_attributes(m_is_resizable);
    connect(windowHandle(), &QWindow::screenChanged, this,
      &Window::on_screen_changed);
    connect(screen(), &QScreen::logicalDotsPerInchChanged, this,
      &Window::on_logical_dots_per_inch_changed);
  }
  return QWidget::event(event);
}

bool Window::nativeEvent(const QByteArray& eventType, void* message,
    long* result) {
  auto msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCACTIVATE) {
    RedrawWindow(msg->hwnd, NULL, NULL, RDW_UPDATENOW);
    *result = -1;
    return true;
  } else if(msg->message == WM_NCCALCSIZE) {
    if(msg->wParam != 0) {
      if(!m_frame_size) {
        m_frame_size = size();
      }
      auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
      auto placement = WINDOWPLACEMENT();
      if(GetWindowPlacement(hwnd, &placement)) {
        auto& rect = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam)->rgrc[0];
        if(IsZoomed(hwnd) ||
            (IsIconic(hwnd) && placement.flags & WPF_RESTORETOMAXIMIZED)) {
          if(auto monitor = MonitorFromRect(
              &placement.rcNormalPosition, MONITOR_DEFAULTTONEAREST)) {
            auto monitor_info = MONITORINFO();
            monitor_info.cbSize = sizeof(monitor_info);
            if(GetMonitorInfoW(monitor, &monitor_info)) {
              rect = monitor_info.rcWork;
            }
          }
        } else {
          auto current_screen = screen();
          auto size = get_system_border_size(
            current_screen->logicalDotsPerInchX(),
            current_screen->logicalDotsPerInchY());
          if(!isVisible()) {
            move(pos() + QPoint(size.width(), 0));
          }
          rect.right -= size.width();
          rect.left += size.width();
          rect.bottom -= size.height();
        }
      }
      *result = 0;
      return true;
    }
  } else if(msg->message == WM_WINDOWPOSCHANGING) {
    auto wp = reinterpret_cast<WINDOWPOS*>(msg->lParam);
    wp->flags |= SWP_NOCOPYBITS;
    *result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT();
    GetWindowRect(reinterpret_cast<HWND>(effectiveWinId()), &window_rect);
    auto x = GET_X_LPARAM(msg->lParam);
    auto y = GET_Y_LPARAM(msg->lParam);
    auto resize_area = RESIZE_AREA();
    if(m_is_resizable) {
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width() &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOMLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width() &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOMRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width() &&
          y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOPLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width() &&
          y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOPRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + resize_area.width()) {
        *result = HTLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - resize_area.width()) {
        *result = HTRIGHT;
        return true;
      }
      if(y < window_rect.bottom &&
          y >= window_rect.bottom - resize_area.height()) {
        *result = HTBOTTOM;
        return true;
      }
      if(y >= window_rect.top &&
          y < window_rect.top + resize_area.height()) {
        *result = HTTOP;
        return true;
      }
    }
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    auto current_screen = screen();
    auto border_size = get_system_border_size(
      current_screen->logicalDotsPerInchX(),
      current_screen->logicalDotsPerInchY());
    if(maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
      mmi->ptMaxTrackSize.x = maximumSize().width() + 2 * border_size.width();
      mmi->ptMaxTrackSize.y = maximumSize().height() + border_size.height();
    } else {
      mmi->ptMaxTrackSize.x = maximumSize().width();
      mmi->ptMaxTrackSize.y = maximumSize().height();
    }
    mmi->ptMinTrackSize.x = minimumSize().width() + 2 * border_size.width();
    mmi->ptMinTrackSize.y = minimumSize().height() + border_size.height();
    return true;
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void Window::set_body(QWidget* body) {
  if(m_body) {
    return;
  }
  m_body = body;
  if(body->maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
    set_window_attributes(false);
  }
  auto& box = *static_cast<Box*>(layout()->itemAt(0)->widget());
  box.get_body()->layout()->addWidget(m_body);
}

void Window::on_highlighted(bool is_match) {
  if(is_match) {
    match(*m_title_bar, Highlighted());
  } else {
    unmatch(*m_title_bar, Highlighted());
  }
}

void Window::on_screen_changed(QScreen* screen) {
  // TODO: Workaround for this change:
  // https://github.com/qt/qtbase/commit/d2fd9b1b9818b3ec88487967e010f66e92952f55
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  auto rect = RECT();
  GetWindowRect(hwnd, &rect);
  SendMessage(hwnd, WM_NCCALCSIZE, TRUE, reinterpret_cast<LPARAM>(&rect));
  connect(screen, &QScreen::logicalDotsPerInchChanged, this,
    &Window::on_logical_dots_per_inch_changed);
}

void Window::on_logical_dots_per_inch_changed() {
  SetWindowPos(reinterpret_cast<HWND>(effectiveWinId()), nullptr, 0, 0, 0, 0,
    SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void Window::set_window_attributes(bool is_resizeable) {
  m_is_resizable = is_resizeable;
  auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
  if(m_is_resizable &&
      maximumSize() != QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
    m_has_maximize_attribute =
      windowFlags() & Qt::WindowMaximizeButtonHint;
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
      WS_THICKFRAME | WS_CAPTION);
  } else if(m_is_resizable) {
    if(!m_has_maximize_attribute) {
      m_has_maximize_attribute =
        windowFlags() & Qt::WindowMaximizeButtonHint;
    }
    if(*m_has_maximize_attribute) {
      setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
      ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
        WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
      ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE) |
        WS_THICKFRAME | WS_CAPTION);
    }
  } else {
    m_has_maximize_attribute =
      windowFlags() & Qt::WindowMaximizeButtonHint;
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX | WS_CAPTION);
  }
  if(m_is_resizable && m_frame_size && size() != m_frame_size) {
    resize(*m_frame_size);
    m_frame_size = none;
  }
  auto window_geometry = frameGeometry();
  auto borderWidth = GetSystemMetrics(SM_CXFRAME) +
    GetSystemMetrics(SM_CXPADDEDBORDER);
  auto borderHeight = GetSystemMetrics(SM_CYFRAME) +
    GetSystemMetrics(SM_CXPADDEDBORDER);
  auto internal_height = layout()->contentsRect().height();
  MoveWindow(hwnd, window_geometry.x() - borderWidth + 1,
    window_geometry.y() + borderHeight + m_title_bar->height() -
      scale_height(3),
    width() + 2 * borderWidth, internal_height + borderHeight, true);
}
