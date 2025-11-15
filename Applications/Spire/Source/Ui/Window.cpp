#include "Spire/Ui/Window.hpp"
#include <QEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QWindow>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TitleBar.hpp"
#include "Spire/Ui/Ui.hpp"

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

  int get_dpi(const QWidget& widget) {
    using GetDpiForWindowProc = int(WINAPI*)(HWND);
    static auto function = [] () -> GetDpiForWindowProc {
      if(auto handle = GetModuleHandle("user32.dll")) {
        return reinterpret_cast<GetDpiForWindowProc>(
          GetProcAddress(handle, "GetDpiForWindow"));
      }
      return nullptr;
    }();
    if(function) {
      return function(reinterpret_cast<HWND>(widget.effectiveWinId()));
    }
    return widget.screen()->logicalDotsPerInch();
  }

  auto get_system_metrics_for_dpi(int index, int dpi) {
    using GetSystemMetricsForDpiProc = int(WINAPI*)(int, UINT);
    static auto function = [] () -> GetSystemMetricsForDpiProc {
      if(auto handle = GetModuleHandle("user32.dll")) {
        return reinterpret_cast<GetSystemMetricsForDpiProc>(
          GetProcAddress(handle, "GetSystemMetricsForDpi"));
      }
      return nullptr;
    }();
    if(function) {
      return function(index, dpi);
    }
    return GetSystemMetrics(index);
  }

  auto get_system_border_size(int dpi) {
    auto padding = get_system_metrics_for_dpi(SM_CXPADDEDBORDER, dpi);
    return QSize(get_system_metrics_for_dpi(SM_CXFRAME, dpi) + padding,
      get_system_metrics_for_dpi(SM_CYFRAME, dpi) + padding);
  }

  int get_bottom_border_offset(HWND hwnd, int bottom_border) {
    auto offset = 0;
    auto window_rect = RECT{0};
    auto extended_bounds = RECT{0};
    if(GetWindowRect(hwnd, &window_rect) &&
        SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS,
          &extended_bounds, sizeof(extended_bounds)))) {
      if(auto bottom = window_rect.bottom - extended_bounds.bottom;
          bottom != 0) {
        offset = bottom_border - 1 - bottom;
      }
    }
    return offset;
  }

  auto make_svg_window_icon(const QString& icon_path) {
    return image_from_svg(icon_path, scale(26, 26), QRect(translate(8, 8),
      scale(10, 10)));
  }

  void show_system_menu(HWND hwnd, int x, int y) {
    auto menu = GetSystemMenu(hwnd, FALSE);
    if(!menu) {
      return;
    }
    auto placement = WINDOWPLACEMENT{sizeof(WINDOWPLACEMENT)};
    GetWindowPlacement(hwnd, &placement);
    auto style = GetWindowLong(hwnd, GWL_STYLE);
    auto minimize_enabled = [&] {
      if(style & WS_MINIMIZEBOX) {
        return MF_ENABLED;
      }
      return MF_GRAYED;
    }();
    auto maximize_enabled = [&] {
      if(style & WS_MAXIMIZEBOX) {
        return MF_ENABLED;
      }
      return MF_GRAYED;
    }();
    if(placement.showCmd == SW_SHOWMAXIMIZED) {
      EnableMenuItem(menu, SC_RESTORE, MF_ENABLED);
      EnableMenuItem(menu, SC_MAXIMIZE, MF_GRAYED);
      EnableMenuItem(menu, SC_MINIMIZE, minimize_enabled);
    } else if(placement.showCmd == SW_SHOWMINIMIZED) {
      EnableMenuItem(menu, SC_RESTORE, MF_ENABLED);
      EnableMenuItem(menu, SC_MAXIMIZE, maximize_enabled);
      EnableMenuItem(menu, SC_MINIMIZE, MF_GRAYED);
    } else {
      EnableMenuItem(menu, SC_RESTORE, MF_GRAYED);
      EnableMenuItem(menu, SC_MAXIMIZE, maximize_enabled);
      EnableMenuItem(menu, SC_MINIMIZE, minimize_enabled);
    }
    if(auto cmd = TrackPopupMenu(menu,
        TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hwnd, nullptr);
        cmd != 0) {
      SendMessage(hwnd, WM_SYSCOMMAND, cmd, 0);
    }
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
    if(!m_is_bottom_border_mismatched) {
      auto offset = get_bottom_border_offset(msg->hwnd,
        get_system_border_size(get_dpi(*this)).height());
      m_is_bottom_border_mismatched = std::abs(offset) > 0;
      SetWindowPos(msg->hwnd, nullptr, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    }
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
          auto border_size = get_system_border_size(get_dpi(*this));
          if(!isVisible()) {
            move(pos() + QPoint(border_size.width(), 0));
          }
          rect.left += border_size.width();
          rect.right -= border_size.width();
          if(m_is_bottom_border_mismatched && *m_is_bottom_border_mismatched) {
            rect.bottom -= 1;
          } else {
            rect.bottom -= border_size.height();
          }
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
      static auto top_left = QPoint(0, 0);
      auto title_bar_pos = m_title_bar->mapToGlobal(top_left);
      auto& title_label = m_title_bar->get_title_label();
      auto title_label_pos = title_label.mapToGlobal(top_left);
      auto icon_rect = QRect(title_bar_pos.x(), title_bar_pos.y(),
        title_label_pos.x() - title_bar_pos.x(), m_title_bar->height());
      if(icon_rect.contains(QPoint(x, y))) {
        *result = HTSYSMENU;
        return true;
      }
      auto title_label_rect = QRect(title_label_pos,
        QSize(title_label.width(), m_title_bar->height()));
      if(title_label_rect.contains(QPoint(x, y))) {
        *result = HTCAPTION;
        return true;
      }
    }
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_GETMINMAXINFO) {
    auto mmi = reinterpret_cast<MINMAXINFO*>(msg->lParam);
    auto border_size = get_system_border_size(get_dpi(*this));
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
  } else if (msg->message == WM_NCRBUTTONUP &&
      (msg->wParam == HTCAPTION || msg->wParam == HTSYSMENU)) {
    show_system_menu(msg->hwnd, GET_X_LPARAM(msg->lParam),
      GET_Y_LPARAM(msg->lParam));
    *result = 0;
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
  auto border_size = get_system_border_size(get_dpi(*this));
  auto internal_height = layout()->contentsRect().height();
  MoveWindow(hwnd, window_geometry.x() - border_size.width() + 1,
    window_geometry.y() + border_size.height() + m_title_bar->height() -
      scale_height(3),
    width() + 2 * border_size.width(), internal_height + border_size.height(),
    true);
}
