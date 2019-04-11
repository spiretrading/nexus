#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include "spire/spire/dimensions.hpp"
#include "spire/spire/resources.hpp"
#include "spire/ui/title_bar.hpp"

using namespace Spire;

#include <QMainWindow>

#include "spire/ui/icon_button.hpp"
#include <qt_windows.h>
#include <windowsx.h>
#include <dwmapi.h>

class CFramelessWindow : public QMainWindow {
  public:

    explicit CFramelessWindow(const QImage& icon, const QImage& unfocused_icon,
      QWidget *parent = 0);

    void setFixedSize(int width, int height);

    void setFixedSize(const QSize& size);

  protected:
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray &eventType, void *message,
      long *result) override;

  private:
    QWidget* m_container;
    TitleBar* m_title_bar;
    int m_resize_area_width;
    bool m_just_maximized;
    bool m_is_resizeable;

    void set_resizeable(bool resizeable);
};

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }
}

CFramelessWindow::CFramelessWindow(const QImage& icon,
    const QImage& unfocused_icon, QWidget *parent)
    : QMainWindow(parent),
      m_resize_area_width(5),
      m_just_maximized(false),
      m_is_resizeable(true),
      m_title_bar(nullptr) {
  setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint |
    Qt::WindowSystemMenuHint);
  m_title_bar = new TitleBar(icon, unfocused_icon, this);
  m_container = new QWidget(this);
  auto container_layout = new QVBoxLayout(m_container);
  container_layout->setSpacing(0);
  container_layout->setContentsMargins(scale_width(1), scale_height(1),
    scale_width(1), scale_height(1));
  container_layout->addWidget(m_title_bar);
  auto label = new QLabel("Test Label", m_container);
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  label->setStyleSheet("background-color: rgb(226, 224, 255);");
  container_layout->addWidget(label);
  setCentralWidget(m_container);

  // TODO: review current and previous cases related to Spire::Window issues
  // and verify that they're fixed with this window

  // TODO: fix the issue where the restored window is partly drawn on the left
  // screen while the window is maximized. This window segment can't be interacted
  // with so maybe it's possible to set its' background color to transparent if it can't be removed

  // TODO: fix issue with winkey + arrow key shortcuts break window. When the window is
  // maximized and winkey + left is pressed, the window is snapped to the left of the
  // screen but the margins aren't added. May have to overwrite the aero snap functionality
  // with regards to keyboard shortcuts.

  set_resizeable(m_is_resizeable);
}

void CFramelessWindow::setFixedSize(int width, int height) {
  setFixedSize({width, height});
}

void CFramelessWindow::setFixedSize(const QSize& size) {
  set_resizeable(false);
  QMainWindow::setFixedSize(size);
}

void CFramelessWindow::set_resizeable(bool resizeable) {
  m_is_resizeable = resizeable;
  if(m_is_resizeable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(winId());
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME |
      WS_CAPTION);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    auto hwnd = reinterpret_cast<HWND>(winId());
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
  }
  m_title_bar->update_window_flags();
  const auto shadow = MARGINS{ 1, 1, 1, 1 };
  DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(winId()),
    &shadow);
}

void CFramelessWindow::changeEvent(QEvent* event) {
  if(event->type() == QEvent::ActivationChange) {
    if(isActiveWindow()) {
      m_container->setStyleSheet("background-color: #A0A0A0;");
    } else {
      m_container->setStyleSheet("background-color: #C8C8C8;");
    }
  }
}

bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message,
    long *result) {
  MSG* msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCCALCSIZE) {
    *result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    auto window_rect = RECT{};
    GetWindowRect(reinterpret_cast<HWND>(winId()), &window_rect);
    auto x = GET_X_LPARAM(msg->lParam);
    auto y = GET_Y_LPARAM(msg->lParam);
    if(m_is_resizeable) {
      if (x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOMLEFT;
        return true;
      }
      if (x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width &&
          y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOMRIGHT;
        return true;
      }
      if (x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width &&
          y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        *result = HTTOPLEFT;
        return true;
      }
      if(x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width &&
          y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        *result = HTTOPRIGHT;
        return true;
      }
      if (x >= window_rect.left &&
          x < window_rect.left + m_resize_area_width) {
        *result = HTLEFT;
        return true;
      }
      if (x < window_rect.right &&
          x >= window_rect.right - m_resize_area_width) {
        *result = HTRIGHT;
        return true;
      }
      if (y < window_rect.bottom &&
          y >= window_rect.bottom - m_resize_area_width) {
        *result = HTBOTTOM;
        return true;
      }
      if (y >= window_rect.top &&
          y < window_rect.top + m_resize_area_width) {
        *result = HTTOP;
        return true;
      }
    }
    QPoint pos = m_title_bar->mapFromGlobal({x, y});
    if (m_title_bar->get_title_label()->geometry().contains(pos)) {
      *result = HTCAPTION;
      return true;
    }
    return false;
  } else if(msg->message == WM_GETMINMAXINFO) {
    if (::IsZoomed(reinterpret_cast<HWND>(winId()))) {
      // TODO: calculate proper margins so this works on every monitor
      //auto margins = QMargins();
      //RECT frame = { 0, 0, 0, 0 };
      //AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

      ////record frame area data
      //double dpr = this->devicePixelRatioF();

      //margins.setLeft(frame.left);
      //margins.setTop(frame.bottom);
      //margins.setRight(frame.right);
      //margins.setBottom(frame.bottom);
      //QMainWindow::setContentsMargins(margins);
      QMainWindow::setContentsMargins(11, 11, 11, 11);
      m_just_maximized = true;
    } else {
      if (m_just_maximized) {
        setContentsMargins(0, 0, 0, 0);
        m_just_maximized = false;
      }
    }
    return false;
  }
  return QMainWindow::nativeEvent(eventType, message, result);
}

namespace {
  auto make_svg_window_icon(const QString& icon_path) {
    return imageFromSvg(icon_path, scale(26, 26),
      QRect(translate(8, 8), scale(10, 10)));
  }
}

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new CFramelessWindow(
    make_svg_window_icon(":icons/spire-icon-black.svg"),
    make_svg_window_icon(":icons/spire-icon-grey.svg"));
  w->setWindowTitle("Spire Test Window");
  w->resize(500, 500);
  w->show();
  application->exec();
}
