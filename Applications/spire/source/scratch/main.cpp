#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <QWidget>
#include <dwmapi.h>
#include <qt_windows.h>
#include <windowsx.h>
#include "spire/spire/resources.hpp" 

using namespace Spire;

class TitleBar : public QWidget {
  public:

    TitleBar(QWidget* parent)
        : QWidget(parent) {
      setFixedHeight(39);
      setStyleSheet("background-color: #F5F5F5;");
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->setSpacing(0);
      m_icon = new QPushButton("@", this);
      m_icon->setFixedSize(39, 39);
      m_icon->setFocusPolicy(Qt::FocusPolicy::NoFocus);
      m_icon->setEnabled(false);
      layout->addWidget(m_icon);
      m_title_label = new QLabel("", this);
      m_title_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      set_title_text_stylesheet(QColor("#000000"));
      layout->addWidget(m_title_label);
      layout->addSpacerItem(new QSpacerItem(12, 10, QSizePolicy::Fixed,
        QSizePolicy::Expanding));
      m_minimize_button = new QPushButton("-", this);
      m_minimize_button->connect(m_minimize_button, &QPushButton::released, this,
        &TitleBar::on_minimize_button_press);
      m_minimize_button->setFixedSize(39, 39);
      m_minimize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
      layout->addWidget(m_minimize_button);
      m_maximize_button = new QPushButton("#", this);
      m_maximize_button->connect(m_maximize_button, &QPushButton::released, this,
        &TitleBar::on_maximize_button_press);
      m_maximize_button->setFixedSize(39, 39);
      m_maximize_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
      layout->addWidget(m_maximize_button);
      m_restore_button = new QPushButton("%", this);
      m_restore_button->connect(m_restore_button, &QPushButton::released, this,
        &TitleBar::on_restore_button_press);
      m_restore_button->setFixedSize(39, 39);
      m_restore_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
      m_restore_button->hide();
      layout->addWidget(m_restore_button);
      m_close_button = new QPushButton("X", this);
      m_close_button->connect(m_close_button, &QPushButton::released, this,
        &TitleBar::on_close_button_press);
      m_close_button->setFixedSize(39, 39);
      m_close_button->setFocusPolicy(Qt::FocusPolicy::NoFocus);
      layout->addWidget(m_close_button);
      connect(window(), &QWidget::windowTitleChanged,
        [=] (auto& title) {on_window_title_change(title);});
      update_window_flags();
    }

    QLabel* get_title_label() const {
      return m_title_label;
    }

    void update_window_flags() {
      m_minimize_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowMinimizeButtonHint));
      m_maximize_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint));
      m_close_button->setVisible(
        window()->windowFlags().testFlag(Qt::WindowCloseButtonHint));
    }

  protected:
    void changeEvent(QEvent* event) {
      if(event->type() == QEvent::ParentChange) {
        connect(window(), &QWidget::windowTitleChanged,
          [=] (auto& title) {on_window_title_change(title);});
      }
    }

    bool eventFilter(QObject* watched, QEvent* event) {
      if(watched == window()) {
        if(event->type() == QEvent::WindowDeactivate) {
          set_title_text_stylesheet(QColor("#A0A0A0"));
        } else if(event->type() == QEvent::WindowActivate) {
          set_title_text_stylesheet(QColor("#000000"));
        } else if(event->type() == QEvent::WindowStateChange) {
          if(window()->isMaximized()) {
            m_maximize_button->hide();
            m_restore_button->show();
          } else {
            if(window()->windowFlags().testFlag(Qt::WindowMaximizeButtonHint)) {
              m_maximize_button->show();
            }
            m_restore_button->hide();
          }
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    void resizeEvent(QResizeEvent* event) {
      on_window_title_change(window()->windowTitle());
    }

  private:
    QPushButton* m_icon;
    QLabel* m_title_label;
    QPushButton* m_minimize_button;
    QPushButton* m_maximize_button;
    QPushButton* m_restore_button;
    QPushButton* m_close_button;

    void on_window_title_change(const QString& title) {
      QFontMetrics metrics(m_title_label->font());
      auto shortened_text = metrics.elidedText(title,
        Qt::ElideRight, m_title_label->width());
      m_title_label->setText(shortened_text);
    }

    void on_minimize_button_press() {
      window()->showMinimized();
    }

    void on_maximize_button_press() {
      window()->showMaximized();
    }

    void on_restore_button_press() {
      window()->showNormal();
    }

    void on_close_button_press() {
      window()->close();
    }

    void set_title_text_stylesheet(const QColor& font_color) {
      m_title_label->setStyleSheet(QString(
        R"(color: %2;
           font-family: Roboto;
           font-size: %1px;)").arg(18).arg(font_color.name()));
    }
};

class Window : public QWidget {
  public:

    explicit Window::Window(QWidget *parent)
        : QWidget(parent),
          m_resize_area_width(5),
          m_is_resizeable(true),
          m_title_bar(nullptr) {
      setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint |
        Qt::WindowSystemMenuHint);
      m_title_bar = new TitleBar(this);
      installEventFilter(m_title_bar);
      auto layout = new QVBoxLayout(this);
      layout->setSpacing(0);
      layout->setContentsMargins(1, 1, 1, 1);
      layout->addWidget(m_title_bar);
      set_resizeable(m_is_resizeable);
    }

    
    void setFixedSize(int width, int height) {
      setFixedSize({width, height});
    }

    void setFixedSize(const QSize& size) {
      set_resizeable(false);
      QWidget::setFixedSize(size);
    }

    void setWindowFlag(Qt::WindowType flag, bool on) {
      QWidget::setWindowFlag(flag, on);
      if(m_title_bar != nullptr) {
        m_title_bar->update_window_flags();
      }
    }

    void setWindowFlags(Qt::WindowFlags type) {
      QWidget::setWindowFlags(type);
      if(m_title_bar != nullptr) {
        m_title_bar->update_window_flags();
      }
    }

  protected:
    void Window::changeEvent(QEvent* event) {
      if(event->type() == QEvent::ActivationChange) {
        if(isActiveWindow()) {
          setStyleSheet("background-color: #A0A0A0;");
        } else {
          setStyleSheet("background-color: #C8C8C8;");
        }
      }
    }

    bool Window::nativeEvent(const QByteArray &eventType, void *message,
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
      }
      return QWidget::nativeEvent(eventType, message, result);
    }

  private:
    TitleBar* m_title_bar;
    int m_resize_area_width;
    bool m_is_resizeable;

    void Window::set_resizeable(bool resizeable) {
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
};

class Dialog : public Window {
  public:

    Dialog(QWidget* parent = nullptr)
      : Window(parent),
      m_result(QDialog::DialogCode::Rejected) {}

    void accept() {
      m_result = QDialog::DialogCode::Accepted;
      close();
    }

    QDialog::DialogCode exec() {
      auto window = static_cast<QWidget*>(parent())->window();
      auto parent_center = QPoint(window->pos().x() + (window->width() / 2),
        window->pos().y() + (window->height() / 2));
      move({parent_center.x() - (width() / 2),
        parent_center.y() - (height() / 2)});
      show();
      while(isVisible()) {
        QApplication::processEvents(QEventLoop::WaitForMoreEvents);
      }
      return m_result;
    }

    void reject() {
      m_result = QDialog::DialogCode::Rejected;
      close();
    }

  private:
    QDialog::DialogCode m_result;
};

class TestDialog : public Dialog {
  public:

    explicit TestDialog(QWidget* parent_window) : Dialog(parent_window) {
      Window::layout()->addWidget(new QLabel("Dialog Label", this));
      resize(300, 300);
    }

  protected:
    void mousePressEvent(QMouseEvent* event) override {
      setFocus();
    }
    void showEvent(QShowEvent* event) override {
      auto parent_geometry = static_cast<QWidget*>(parent())->geometry();
      move(parent_geometry.center().x() - (width() / 2),
        parent_geometry.center().y() - (height() / 2));
    }
};

class TestWindow : public Window {
  public:

    TestWindow(QWidget* parent = nullptr)
        : Window(parent) {
      auto label = new QLabel("Test Label", this);
      layout()->addWidget(label);
      auto button = new QPushButton("Show Dialog", this);
      connect(button, &QPushButton::released, this, &TestWindow::show_dialog);
      button->setStyleSheet("background-color: white;");
      button->setFixedHeight(30);
      layout()->addWidget(button);
    }

  private:
    void show_dialog() {
      TestDialog dialog(this);
      dialog.exec();
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto w = new TestWindow();
  w->resize(500, 500);
  w->show();
  application->exec();
}
