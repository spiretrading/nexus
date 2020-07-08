#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/ScrollArea.hpp"
#include "Spire/Ui/Ui.hpp"

class DropDownWindow : public QWidget {
  public:

    DropDownWindow(QWidget* parent = nullptr)
        : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
          m_widget(nullptr) {
      setAttribute(Qt::WA_ShowWithoutActivating);
      setAttribute(Qt::WA_TranslucentBackground);
      m_shadow = new DropShadow(true, false, this);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      parent->installEventFilter(this);
      parent->window()->installEventFilter(this);
    }

    void set_widget(QWidget* widget) {
      if(m_widget != nullptr) {
        return;
      }
      m_widget = widget;
      m_widget->setParent(this);
      m_widget->installEventFilter(this);
      layout()->addWidget(m_widget);
    }

  protected:
    bool event(QEvent* event) override {
      if(event->type() == QEvent::WindowDeactivate) {
        hide();
      }
      return QWidget::event(event);
    }
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == m_widget) {
        if(event->type() == QEvent::Resize) {
          resize(m_widget->width() + 2, m_widget->height() + 2);
        }
      } else if(watched == parent()) {
        if(event->type() == QEvent::Move) {
          move_to_parent();
        } else if(event->type() == QEvent::FocusOut) {
          hide();
        } else if(event->type() == QEvent::Wheel) {
          hide();
        }
      } else if(watched == static_cast<QWidget*>(parent())->window()) {
        if(event->type() == QEvent::WindowDeactivate && !isActiveWindow()) {
          hide();
        } else if(event->type() == QEvent::Move) {
          move_to_parent();
        }
      }
      return QWidget::eventFilter(watched, event);
    }
    void keyPressEvent(QKeyEvent* event) override {
      if(event->key() == Qt::Key_Escape) {
        hide();
      }
    }
    void paintEvent(QPaintEvent* event) override {
      QWidget::paintEvent(event);
      auto painter = QPainter(this);
      painter.setPen("#4B23A0");
      painter.drawLine(0, 0, static_cast<QWidget*>(parent())->width(), 0);
    }

  private:
    QWidget* m_widget;
    DropShadow* m_shadow;

    void move_to_parent() {
      auto x_pos = static_cast<QWidget*>(parent())->mapToGlobal(
        geometry().bottomLeft()).x();
      auto y_pos = static_cast<QWidget*>(parent())->mapToGlobal(
        frameGeometry().bottomLeft()).y();
      move(x_pos, y_pos + 1);
      move(100, 100);
      qDebug() << pos();
      raise();
    }
};

class DropDownItem : public QWidget {
  public:

    DropDownItem(const QString& text,
      QWidget* parent = nullptr)
      : QWidget(parent),
        m_text(text) {}

    const QVariant& get_text() const {
      return m_text;
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
      auto painter = QPainter(this);
      painter.fillRect(rect(), Qt::red);
      painter.setPen(Qt::black);
      painter.drawText(rect().topLeft(), m_text);
    }

  private:
    QString m_text;
};

class DropDownList : public DropDownWindow {
  public:

    DropDownList(std::vector<DropDownItem*> items,
        QWidget* parent = nullptr) : DropDownWindow(parent) {
      auto scroll_area = new ScrollArea(this);
      scroll_area->setWidgetResizable(true);
      auto main_widget = new QWidget(this);
      auto layout = new QVBoxLayout(main_widget);
      for(auto item : items) {
        layout->addWidget(item);
      }
      scroll_area->setWidget(main_widget);
      set_widget(scroll_area);
      //setFixedHeight(scale_height(20) *
      //  std::min(static_cast<int>(items.size()), 5));
      // TODO: make this dynamic based on some criteria
      //setFixedWidth(scale_width(100));
      scroll_area->setFixedHeight(500);
      scroll_area->setFixedWidth(500);
    }
};

class DropDownMenu : public QWidget {
  public:

    explicit DropDownMenu(const std::vector<QString>& items,
        QWidget* parent)
        : QWidget(parent),
          m_dropdown_image(imageFromSvg(":/Icons/arrow-down.svg", scale(6, 4))) {
      setFocusPolicy(Qt::StrongFocus);
      if(!items.empty()) {
        m_current_text = items.front();
      }
      auto widget_items = std::vector<DropDownItem*>();
      widget_items.reserve(items.size());
      for(auto& item : items) {
        widget_items.push_back(new DropDownItem(item, this));
      }
      m_menu_list = new DropDownList(widget_items, this);
    }

    void set_current_text(const QString& text);

    void set_items(const std::vector<QString>& items);

    const QString& get_text() const;

  protected:
    void mousePressEvent(QMouseEvent* event) override {
      if(event->button() == Qt::LeftButton) {
        if(!m_menu_list->isVisible()) {
          m_menu_list->show();
        } else {
          m_menu_list->hide();
        }
      }
    }
    void paintEvent(QPaintEvent* event) override {
      // TODO: move to anon namespace
      auto PADDING = [] { return scale_width(8); };
      auto painter = QPainter(this);
      if(hasFocus() || m_menu_list->hasFocus()) {
        painter.fillRect(event->rect(), QColor("#4B23A0"));
      } else {
        painter.fillRect(event->rect(), QColor("#C8C8C8"));
      }
      painter.fillRect(1, 1, width() - 2, height() - 2, Qt::white);
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      painter.setFont(font);
      auto metrics = QFontMetrics(font);
      painter.drawText(QPoint(PADDING(),
        (height() / 2) + (metrics.ascent() / 2) - 1),
        metrics.elidedText(m_current_text, Qt::ElideRight,
        width() - (PADDING() * 3)));
      painter.drawImage(
        QPoint(width() - (m_dropdown_image.width() + PADDING()),
        scale_height(11)), m_dropdown_image);
    }

  private:
    QString m_current_text;
    QImage m_dropdown_image;
    DropDownList* m_menu_list;
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Eidolon Systems Ltd"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto test_window = new QWidget();
  auto layout = new QHBoxLayout(test_window);
  auto dropdown1 = new ::DropDownMenu({"One", "Two", "Three"}, test_window);
  dropdown1->setFixedSize(scale(100, 28));
  layout->addWidget(dropdown1);
  test_window->resize(800, 600);
  test_window->show();
  application->exec();
}
