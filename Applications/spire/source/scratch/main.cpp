#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "spire/spire/resources.hpp"

#include "spire/ui/title_bar.hpp"
#include "spire/spire/dimensions.hpp"

using namespace Spire;

#include <QObject>
#include <QMainWindow>

#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>

class CFramelessWindow : public QMainWindow
{
public:
    explicit CFramelessWindow(const QImage& icon, const QImage& unfocused_icon,
      QWidget *parent = 0);
public:

    //if resizeable is set to false, then the window can not be resized by mouse
    //but still can be resized programtically
    void setResizeable(bool resizeable=true);
    bool isResizeable(){return m_bResizeable;}

    //set border width, inside this aera, window can be resized by mouse
    void setResizeableAreaWidth(int width = 5);

    void setCentralWidget(QWidget* widget);

    //generally, we can add widget say "label1" on titlebar, and it will cover the titlebar under it
    //as a result, we can not drag and move the MainWindow with this "label1" again
    //we can fix this by add "label1" to a ignorelist, just call addIgnoreWidget(label1)
    void addIgnoreWidget(QWidget* widget);

protected:
    //bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

//private slots:
//    void onTitleBarDestroyed();
public:
    //void setContentsMargins(const QMargins &margins);
    //void setContentsMargins(int left, int top, int right, int bottom);
    //QMargins contentsMargins() const;
    QRect contentsRect() const;
    //void getContentsMargins(int *left, int *top, int *right, int *bottom) const;



//public slots:
//    void showFullScreen();
private:
    QList<QWidget*> m_whiteList;
    int m_borderWidth;

    QMargins m_frames;
    bool m_bJustMaximized;

    bool m_bResizeable;

    //IconButton* m_icon;
    //QImage m_default_icon;
    //QImage m_unfocused_icon;
    //QLabel* m_title_label;
    //IconButton* m_minimize_button;
    //IconButton* m_maximize_button;
    //IconButton* m_restore_button;
    //IconButton* m_close_button;
    TitleBar* m_title_bar;
};

#include <QPoint>
#include <QSize>

#define NOMINMAX
#include <algorithm>
namespace Gdiplus
{
  using std::min;
  using std::max;
}

#include "spire/ui/icon_button.hpp"
#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined

#include <gdiplus.h>
#include <GdiPlusColor.h>

namespace {
  auto ICON_SIZE() {
    return scale(26, 26);
  }
}

CFramelessWindow::CFramelessWindow(const QImage& icon,
    const QImage& unfocused_icon, QWidget *parent)
    : QMainWindow(parent),
      m_borderWidth(5),
      m_bJustMaximized(false),
      m_bResizeable(true),
      m_title_bar(nullptr) {
  setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint |
    Qt::WindowSystemMenuHint);
  // TODO: how to disable maximize button (including system menu maximize item)?
  //setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
  setResizeable(m_bResizeable);
  m_title_bar = new TitleBar(icon, unfocused_icon, this);
  addIgnoreWidget(m_title_bar->get_title_label());
  auto c = new QWidget(this);
  auto c_layout = new QVBoxLayout(c);
  //setTitleBar(m_title_bar);
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
  // is it possible that the widget that window() return changes?
  window()->installEventFilter(m_title_bar);

  c->setStyleSheet("background-color: red;");

  // TODO: review current and previous cases related to Spire::Window issues
  // and verify that they're fixed with this window

  // TODO: fix the issue where the restored window is partly drawn on the left
  // screen while the window is maximized. This window segment can't be interacted
  // with so maybe it's possible to set its' background color to transparent if it can't be removed
  //window()->setStyleSheet("background-color: transparent;");
  //c->setStyleSheet("background-color: transparent;");


  // why doesn't this window receive resize events in resizeEvent()?
  window()->installEventFilter(this);
}

void CFramelessWindow::setResizeable(bool resizeable) {
    bool visible = isVisible();
    m_bResizeable = resizeable;
    if(m_bResizeable) {
        //setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        //this line will get titlebar/thick frame/Aero back, which is exactly what we want
        //we will get rid of titlebar and thick frame again in nativeEvent() later
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }

    //we better left 1 piexl width of border untouch, so OS can draw nice shadow around it
    const MARGINS shadow = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
    setVisible(visible);
}

void CFramelessWindow::setResizeableAreaWidth(int width) {
  if(1 > width) {
    width = 1;
  }
  m_borderWidth = width;
}

void CFramelessWindow::setCentralWidget(QWidget* widget) {
  if(centralWidget() != nullptr) {
    widget->removeEventFilter(this);
  }
  widget->installEventFilter(this);
  QMainWindow::setCentralWidget(widget);
}

//void CFramelessWindow::onTitleBarDestroyed()
//{
//    if (m_titlebar == QObject::sender())
//    {
//        m_titlebar = Q_NULLPTR;
//    }
//}

void CFramelessWindow::addIgnoreWidget(QWidget* widget) {
  if(!widget) {
    return;
  }
  if(m_whiteList.contains(widget)) {
    return;
  }
  m_whiteList.append(widget);
}

bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message, long *result) {
    MSG* msg = reinterpret_cast<MSG*>(message);
    switch (msg->message)
    {
    case WM_NCCALCSIZE:
    {
      //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
      // why does it crash when I remove this?
      *result = 0;
      return true;
    }
    case WM_NCHITTEST:
    {
      *result = 0;
      const LONG border_width = m_borderWidth;
      RECT winrect;
      GetWindowRect(HWND(winId()), &winrect);
      long x = GET_X_LPARAM(msg->lParam);
      long y = GET_Y_LPARAM(msg->lParam);
      if(m_bResizeable) {
        bool resizeWidth = minimumWidth() != maximumWidth();
        bool resizeHeight = minimumHeight() != maximumHeight();
        if(resizeWidth) {
          //left border
          if (x >= winrect.left && x < winrect.left + border_width) {
            *result = HTLEFT;
          }
          //right border
          if (x < winrect.right && x >= winrect.right - border_width) {
            *result = HTRIGHT;
          }
        }
        if(resizeHeight) {
          //bottom border
          if (y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOM;
          }
          //top border
          if (y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOP;
          }
        }
        if(resizeWidth && resizeHeight) {
          //bottom left corner
          if (x >= winrect.left && x < winrect.left + border_width &&
              y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOMLEFT;
          }
          //bottom right corner
          if (x < winrect.right && x >= winrect.right - border_width &&
              y < winrect.bottom && y >= winrect.bottom - border_width) {
            *result = HTBOTTOMRIGHT;
          }
          //top left corner
          if (x >= winrect.left && x < winrect.left + border_width &&
              y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOPLEFT;
          }
          //top right corner
          if(x < winrect.right && x >= winrect.right - border_width &&
              y >= winrect.top && y < winrect.top + border_width) {
            *result = HTTOPRIGHT;
          }
        }
      }
      if (0 != *result) return true;
      //*result still equals 0, that means the cursor locate OUTSIDE the frame area
      //but it may locate in titlebar area
      if (!m_title_bar) return false;
      //support highdpi
      double dpr = this->devicePixelRatioF();
      QPoint pos = m_title_bar->mapFromGlobal(QPoint(x/dpr,y/dpr));
      if (!m_title_bar->rect().contains(pos)) return false;
      QWidget* child = m_title_bar->childAt(pos);
      if(!child) {
        *result = HTCAPTION;
        return true;
      } else {
        if (m_whiteList.contains(child)) {
          *result = HTCAPTION;
          return true;
        }
      }
      return false;
    } //end case WM_NCHITTEST
    case WM_GETMINMAXINFO:
    {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

            ////record frame area data
            //double dpr = this->devicePixelRatioF();

            //m_frames.setLeft(abs(frame.left)/dpr+0.5);
            //m_frames.setTop(abs(frame.bottom)/dpr+0.5);
            //m_frames.setRight(abs(frame.right)/dpr+0.5);
            //m_frames.setBottom(abs(frame.bottom)/dpr+0.5);

            setContentsMargins(11, 11, 11, 11);
            m_bJustMaximized = true;
            //*result = 0;
            //return true;
        }else {
            if (m_bJustMaximized)
            {
                //QMainWindow::setContentsMargins(m_margins);
                //after window back to normal size from maximized state
                //a twinkle will happen, to avoid this twinkle
                //repaint() is important used just before the window back to normal
                setContentsMargins(0, 0, 0, 0);
                repaint();
                //m_frames = QMargins();
                m_bJustMaximized = false;
            }
        }
        return false;
    }
    default:
        return QMainWindow::nativeEvent(eventType, message, result);
    }
}

//void CFramelessWindow::setContentsMargins(const QMargins &margins)
//{
//    QMainWindow::setContentsMargins(margins);
//    //m_margins = margins;
//}
//void CFramelessWindow::setContentsMargins(int left, int top, int right, int bottom)
//{
//    //QMainWindow::setContentsMargins(left+m_frames.left(),\
//    //                                top+m_frames.top(), \
//    //                                right+m_frames.right(), \
//    //                                bottom+m_frames.bottom());
//    //m_margins.setLeft(left);
//    //m_margins.setTop(top);
//    //m_margins.setRight(right);
//    //m_margins.setBottom(bottom);
//}
//QMargins CFramelessWindow::contentsMargins() const
//{
//    QMargins margins = QMainWindow::contentsMargins();
//    //margins -= m_frames;
//    return margins;
//}
//void CFramelessWindow::getContentsMargins(int *left, int *top, int *right, int *bottom) const
//{
//    QMainWindow::getContentsMargins(left,top,right,bottom);
//    if (!(left&&top&&right&&bottom)) return;
//    if (isMaximized())
//    {
//        //*left -= m_frames.left();
//        //*top -= m_frames.top();
//        //*right -= m_frames.right();
//        //*bottom -= m_frames.bottom();
//    }
//}

QRect CFramelessWindow::contentsRect() const
{
    QRect rect = QMainWindow::contentsRect();
    int width = rect.width();
    int height = rect.height();
    rect.setLeft(rect.left());
    rect.setTop(rect.top());
    rect.setWidth(width);
    rect.setHeight(height);
    return rect;
}
//void CFramelessWindow::showFullScreen()
//{
//    if (isMaximized())
//    {
//        QMainWindow::setContentsMargins(m_margins);
//        m_frames = QMargins();
//    }
//    QMainWindow::showFullScreen();
//}

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
  w->show();
  application->exec();
}
