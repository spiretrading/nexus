#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QResizeEvent>
#include "spire/spire/resources.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/icon_button.hpp"
#include "spire/ui/title_bar.hpp"
#include "spire/ui/window.hpp"

using namespace spire;

class drop_shadow_widget : public QWidget {
  public:
    drop_shadow_widget(QMargins margins, QWidget* parent = nullptr)
        : QWidget(parent),
          m_margins(std::move(margins)) {
      setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
      setAttribute(Qt::WA_TranslucentBackground);
      setAttribute(Qt::WA_ShowWithoutActivating);
      m_inner_widget = new QWidget(this);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins(margins);
      layout->setSpacing(0);
      layout->addWidget(m_inner_widget);
      auto drop_shadow = new QGraphicsDropShadowEffect(this);
      drop_shadow->setBlurRadius(120);
      drop_shadow->setXOffset(0);
      drop_shadow->setYOffset(0);
      drop_shadow->setColor(QColor(255, 0, 0, 200));
      m_inner_widget->setGraphicsEffect(drop_shadow);
      //m_inner_widget->setStyleSheet("background-color: red;");
      parent->installEventFilter(this);
      m_parent = parent;
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == parent()) {
        if(event->type() == QEvent::Move) {
          auto top_left = static_cast<QWidget*>(parent())->frameGeometry().topLeft();
          auto x_pos = top_left.x();
          auto y_pos = top_left.y();
          move(x_pos - m_margins.left(), y_pos - m_margins.top());
        }
        if(event->type() == QEvent::Resize) {
          auto parent_size = static_cast<QWidget*>(parent())->size();
          resize(
            parent_size.width() + m_margins.left() + m_margins.right(),
            parent_size.height() + m_margins.top() + m_margins.bottom());
        }
        if(event->type() == QEvent::Show) {
          show();
        }
      }
      return QWidget::eventFilter(watched, event);
    }

    void showEvent(QShowEvent* event) override {
      auto parent_widget = static_cast<QWidget*>(parent());
      auto top_left = parent_widget->mapToGlobal(QPoint(0, 0));
      auto x_pos = top_left.x();
      auto y_pos = top_left.y();
      move(x_pos - m_margins.left(), y_pos - m_margins.top());
    }

  private:
    QMargins m_margins;
    QWidget* m_parent;
    QWidget* m_inner_widget;
};

class test_window : public QWidget {
  public:
    test_window(QWidget* parent = nullptr)
        : QWidget(parent) {
      setWindowFlags(Qt::Window | Qt::NoDropShadowWindowHint);
      setBaseSize(600, 480);
      auto drop_shadow = new drop_shadow_widget(QMargins(0, 0, 0, 100), this);
      setStyleSheet("background-color: aqua;");
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins(50, 50, 50, 50);
      auto widget = new QWidget(this);
      widget->setFixedSize(300, 240);
      widget->setStyleSheet("background-color: blue;");
      layout->addWidget(widget);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  test_window test;
  test.show();
  application->exec();
}
