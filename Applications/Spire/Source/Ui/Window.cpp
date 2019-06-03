#include "Spire/Ui/Window.hpp"
#include <QEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TitleBar.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }

  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26),
      QRect(translate(8, 8), scale(10, 10)));
  }
}

Window::Window(QWidget* parent)
    : QWidget(parent),
      m_resize_area_width(5),
      m_is_resizeable(true),
      m_title_bar(nullptr) {
  setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint |
    Qt::WindowSystemMenuHint);
  m_title_bar = new TitleBar(this);
  set_svg_icon(":icons/spire-icon-black.svg", ":icons/spire-icon-grey.svg");
  installEventFilter(m_title_bar);
  auto layout = new QVBoxLayout(this);
  layout->setSpacing(0);
  layout->setContentsMargins(scale_width(1), scale_height(1),
    scale_width(1), scale_height(1));
  layout->addWidget(m_title_bar);
}

void Window::set_icon(const QImage& icon) {
  m_title_bar->set_icon(icon);
}

void Window::set_icon(const QImage& icon, const QImage& unfocused_icon) {
  m_title_bar->set_icon(icon, unfocused_icon);
}

void Window::set_svg_icon(const QString& icon_path) {
  set_icon(make_svg_window_icon(icon_path));
}

void Window::set_svg_icon(const QString& icon_path,
    const QString& unfocused_icon_path) {
  set_icon(make_svg_window_icon(icon_path),
    make_svg_window_icon(unfocused_icon_path));
}

void Window::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    if(isActiveWindow()) {
      setStyleSheet("background-color: #A0A0A0;");
    } else {
      setStyleSheet("background-color: #C8C8C8;");
    }
  }
}

void Window::closeEvent(QCloseEvent* event) {
  m_closed_signal();
}

bool Window::event(QEvent* event) {
  if(event->type() == QEvent::WinIdChange) {
    set_resizeable(m_is_resizeable);
  }
  return QWidget::event(event);
}

bool Window::nativeEvent(const QByteArray& eventType, void* message,
    long* result) {
  auto msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCCALCSIZE) {
    *result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT{};
    GetWindowRect(reinterpret_cast<HWND>(effectiveWinId()), &window_rect);
    auto x = GET_X_LPARAM(msg->lParam);
    auto y = GET_Y_LPARAM(msg->lParam);
    if(m_is_resizeable) {
      if(x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENESW);
        *result = HTBOTTOMLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENWSE);
        *result = HTBOTTOMRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width &&
          y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENWSE);
        *result = HTTOPLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width &&
          y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENESW);
        *result = HTTOPRIGHT;
        return true;
      }
      if(x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZEWE);
        *result = HTLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZEWE);
        *result = HTRIGHT;
        return true;
      }
      if(y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENS);
        *result = HTBOTTOM;
        return true;
      }
      if(y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        m_cursor = LoadCursor(NULL, IDC_SIZENS);
        *result = HTTOP;
        return true;
      }
    }
    auto pos = m_title_bar->mapFromGlobal({x, y});
    if(m_title_bar->get_title_label()->geometry().contains(pos)) {
      m_cursor = LoadCursor(NULL, IDC_ARROW);
      *result = HTCAPTION;
      return true;
    }
    m_cursor = LoadCursor(NULL, IDC_ARROW);
    *result = HTCLIENT;
    return true;
  } else if(msg->message == WM_SIZE) {
    if(msg->wParam == SIZE_MAXIMIZED) {
      auto abs_pos = mapToGlobal(m_title_bar->pos());
      auto pos = QGuiApplication::screenAt(mapToGlobal({window()->width() / 2,
        window()->height() / 2}))->geometry().topLeft();
      pos = QPoint(std::abs(abs_pos.x() - pos.x()),
        std::abs(abs_pos.y() - pos.y()));
      setContentsMargins(pos.x(), pos.y(), pos.x(), pos.y());
    } else if(msg->wParam == SIZE_RESTORED) {
      setContentsMargins({});
    }
  } else if(msg->message == WM_SETCURSOR) {
    // TODO: remove this when WM_NCHITTEST's HTCLIENT result behaves properly.
    SetCursor(m_cursor);
    return true;
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void Window::resize_body(const QSize& size) {
  resize({size.width(), size.height() + m_title_bar->height()});
}

void Window::set_fixed_body_size(const QSize& size) {
  set_resizeable(false);
  setFixedSize({size.width(), size.height() + m_title_bar->height()});
}

void Window::set_resizeable(bool resizeable) {
  m_is_resizeable = resizeable;
  if(m_is_resizeable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    ::SetWindowLong(hwnd, GWL_STYLE, ::GetWindowLong(hwnd, GWL_STYLE)
      | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    ::SetWindowLong(hwnd, GWL_EXSTYLE , ::GetWindowLong(hwnd, GWL_EXSTYLE) |
      WS_EX_TRANSPARENT);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(effectiveWinId());
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX | WS_CAPTION);
  }
  const auto shadow = MARGINS{ 1, 1, 1, 1 };
  DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(effectiveWinId()),
    &shadow);
}
