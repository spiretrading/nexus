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

    // if resizeable is set to false, then the window can not be resized by mouse
    // but still can be resized programmatically
    void setResizeable(bool resizeable = true);

    bool isResizeable() { return m_is_resizeable; }

  protected:
    bool nativeEvent(const QByteArray &eventType, void *message,
      long *result) override;

  private:
    TitleBar* m_title_bar;
    int m_resize_area_width;
    bool m_just_maximized;
    bool m_is_resizeable;
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
  // TODO: how to deal with max size windows?
  // use QWIDGETSIZE_MAX to determine if a window has a maximum size set or not

  // TODO: how to disable maximize button (including system menu maximize item)?
  //setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  m_title_bar = new TitleBar(icon, unfocused_icon, this);
  auto c = new QWidget(this);
  auto c_layout = new QVBoxLayout(c);
  c_layout->setSpacing(0);
  c_layout->setContentsMargins(scale_width(1), scale_height(1), scale_width(1),
    scale_height(1));
  c_layout->addWidget(m_title_bar);
  auto label = new QLabel("Test Label", c);
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  label->setStyleSheet("background-color: rgb(226, 224, 255);");
  c_layout->addWidget(label);
  setCentralWidget(c);
  // TODO: why doesn't this work when installed directly in the TitleBar constructor?
  // is it possible that the widget that window() returns changes?
  window()->installEventFilter(m_title_bar);

  // TODO: window border color on active/inactive
  c->setStyleSheet("background-color: red;");

  // TODO: review current and previous cases related to Spire::Window issues
  // and verify that they're fixed with this window

  // TODO: fix the issue where the restored window is partly drawn on the left
  // screen while the window is maximized. This window segment can't be interacted
  // with so maybe it's possible to set its' background color to transparent if it can't be removed
}

void CFramelessWindow::setResizeable(bool resizeable) {
  m_is_resizeable = resizeable;
  if(m_is_resizeable) {
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    if(m_title_bar != nullptr) {
      m_title_bar->update_window_flags();
    }
    //this line will get titlebar/thick frame/Aero back, which is exactly what we want
    //we will get rid of titlebar and thick frame again in nativeEvent() later
    HWND hwnd = (HWND)this->winId();
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
  } else {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    if(m_title_bar != nullptr) {
      m_title_bar->update_window_flags();
    }
    HWND hwnd = (HWND)this->winId();
    auto style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
  }
  // TODO: why is this required?
  //we better left 1 pixel width of border untouched, so OS can draw nice shadow around it
  const MARGINS shadow = { 1, 1, 1, 1 };
  DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
}

bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
  MSG* msg = reinterpret_cast<MSG*>(message);
  if(msg->message == WM_NCCALCSIZE) {
    //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
    // why does it crash when I remove this?
    *result = 0;
    return true;
  } else if(msg->message == WM_NCHITTEST) {
    RECT window_rect;
    GetWindowRect(HWND(winId()), &window_rect);
    long x = GET_X_LPARAM(msg->lParam);
    long y = GET_Y_LPARAM(msg->lParam);
    if(m_is_resizeable) {
      bool resizeWidth = minimumWidth() != maximumWidth();
      bool resizeHeight = minimumHeight() != maximumHeight();
      if(resizeWidth && resizeHeight) {
        if (x >= window_rect.left && x < window_rect.left + m_resize_area_width &&
            y < window_rect.bottom && y >= window_rect.bottom - m_resize_area_width) {
          *result = HTBOTTOMLEFT;
          return true;
        }
        if (x < window_rect.right && x >= window_rect.right - m_resize_area_width &&
            y < window_rect.bottom && y >= window_rect.bottom - m_resize_area_width) {
          *result = HTBOTTOMRIGHT;
          return true;
        }
        if (x >= window_rect.left && x < window_rect.left + m_resize_area_width &&
            y >= window_rect.top && y < window_rect.top + m_resize_area_width) {
          *result = HTTOPLEFT;
          return true;
        }
        if(x < window_rect.right && x >= window_rect.right - m_resize_area_width &&
            y >= window_rect.top && y < window_rect.top + m_resize_area_width) {
          *result = HTTOPRIGHT;
          return true;
        }
      }
      if(resizeWidth) {
        if (x >= window_rect.left && x < window_rect.left + m_resize_area_width) {
          *result = HTLEFT;
          return true;
        }
        if (x < window_rect.right && x >= window_rect.right - m_resize_area_width) {
          *result = HTRIGHT;
          return true;
        }
      }
      if(resizeHeight) {
        if (y < window_rect.bottom && y >= window_rect.bottom - m_resize_area_width) {
          *result = HTBOTTOM;
          return true;
        }
        if (y >= window_rect.top && y < window_rect.top + m_resize_area_width) {
          *result = HTTOP;
          return true;
        }
      }
    }
    QPoint pos = m_title_bar->mapFromGlobal({x, y});
    // TODO: review if there might not be a better way to check this
    // or, can title bar handle dragging?
    if (m_title_bar->get_title_label()->rect().contains(pos)) {
      *result = HTCAPTION;
      return true;
    }
    return false;
  } else if(msg->message == WM_GETMINMAXINFO) {
    if (window()->isMaximized()) {
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
  w->resize(400, 600);
  w->setResizeable(false);
  w->show();
  application->exec();
}
