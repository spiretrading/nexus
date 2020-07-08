#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
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
      if(event->type() == QEvent::WindowDeactivate &&
          !isAncestorOf(focusWidget())) {
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
        } else if(event->type() == QEvent::FocusOut &&
            !isAncestorOf(focusWidget())) {
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

    DropDownItem(const QVariant& value,
        QWidget* parent = nullptr)
        : QWidget(parent),
          m_value(value),
          m_is_highlighted(false) {
      setAttribute(Qt::WA_Hover);
    }

    const QVariant& get_value() const {
      return m_value;
    }

    void set_highlight() {
      m_is_highlighted = true;
      update();
    }

    void reset_highlight() {
      m_is_highlighted = false;
      update();
    }

  protected:
    void paintEvent(QPaintEvent* event) override {
      auto painter = QPainter(this);
      if(underMouse() || m_is_highlighted) {
        painter.fillRect(rect(), QColor("#F2F2FF"));
      } else {
        painter.fillRect(rect(), Qt::white);
      }
      painter.setPen(Qt::black);
      painter.drawText(10, 10, m_item_delegate.displayText(m_value));
    }

  private:
    QVariant m_value;
    bool m_is_highlighted;
    CustomVariantItemDelegate m_item_delegate;
};

class DropDownList : public DropDownWindow {
  public:

    DropDownList(std::vector<DropDownItem*> items,
        QWidget* parent = nullptr)
        : DropDownWindow(parent),
          m_max_displayed_items(5) {
      auto scroll_area = new ScrollArea(this);
      scroll_area->setWidgetResizable(true);
      auto main_widget = new QWidget(this);
      m_layout = new QVBoxLayout(main_widget);
      m_layout->setContentsMargins({});
      m_layout->setSpacing(0);
      scroll_area->setWidget(main_widget);
      set_widget(scroll_area);
      set_items(items);
      parent->installEventFilter(this);
    }

    void set_items(std::vector<DropDownItem*> items) {
      while(auto item = m_layout->takeAt(0)) {
        delete item->widget();
        delete item;
      }
      for(auto item : items) {
        m_layout->addWidget(item);
      }
      if(m_layout->count() > 0) {
        setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
          m_layout->itemAt(0)->widget()->height());
      } else {
        hide();
      }
    }

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
      if(watched == parent()) {
        if(event->type() == QEvent::KeyPress) {
          auto e = static_cast<QKeyEvent*>(event);
          if(e->key() == Qt::Key_Up) {
            focus_previous();
          } else if(e->key() == Qt::Key_Down) {
            focus_next();
          }
        }
      }
      return DropDownWindow::eventFilter(watched, event);
    }
    void keyPressEvent(QKeyEvent* event) override {
      if(event->key() == Qt::Key_Up) {
        focus_previous();
      } else if(event->key() == Qt::Key_Down) {
        focus_next();
      }
    }

  private:
    int m_max_displayed_items;
    QVBoxLayout* m_layout;
    boost::optional<int> m_highlight_index;

    DropDownItem* get_widget(int index) {
      return static_cast<DropDownItem*>(m_layout->itemAt(index)->widget());
    }
    void focus_next() {
      if(m_layout->count() == 0) {
        return;
      }
      if(!m_highlight_index) {
        m_highlight_index = 0;
        get_widget(*m_highlight_index)->set_highlight();
        return;
      }
      get_widget(*m_highlight_index)->reset_highlight();
      m_highlight_index = (++(*m_highlight_index)) % m_layout->count();
      get_widget(*m_highlight_index)->set_highlight();
    }
    void focus_previous() {
      if(m_layout->count() == 0) {
        return;
      }
      if(!m_highlight_index) {
        m_highlight_index = m_layout->count() - 1;
        get_widget(*m_highlight_index)->set_highlight();
        return;
      }
      get_widget(*m_highlight_index)->reset_highlight();
      --(*m_highlight_index);
      if(*m_highlight_index < 0) {
        m_highlight_index = m_layout->count() - 1;
      }
      get_widget(*m_highlight_index)->set_highlight();
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
      set_items(items);
    }

    void set_list_width(int width) {
      m_menu_list->setFixedWidth(width);
    }

    void set_current_text(const QString& text);

    void set_items(const std::vector<QString>& items) {
      auto widget_items = std::vector<DropDownItem*>();
      widget_items.reserve(items.size());
      for(auto& item : items) {
        auto item_widget = new DropDownItem(item, this);
        item_widget->setFixedHeight(scale_height(20));
        widget_items.push_back(item_widget);
      }
      m_menu_list = new DropDownList(widget_items, this);
    }

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
  auto dropdown1 = new ::DropDownMenu({"One", "Two", "Three", "Four", "Five",
    "Six", "Seven"}, test_window);
  dropdown1->setFixedSize(scale(100, 28));
  layout->addWidget(dropdown1);
  test_window->resize(800, 600);
  test_window->show();
  application->exec();
}
