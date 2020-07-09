#include <QApplication>
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

#include <QApplication>
#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
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
      qDebug() << parent;
      setAttribute(Qt::WA_ShowWithoutActivating);
      setAttribute(Qt::WA_TranslucentBackground);
      m_shadow = new DropShadow(true, false, this);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins(scale_width(1), scale_height(1),
        scale_width(1), scale_height(1));
      parent->installEventFilter(this);
      parent->window()->installEventFilter(this);
      qApp->installEventFilter(this);
    }

    void set_widget(QWidget* widget) {
      if(m_widget != nullptr) {
        return;
      }
      m_widget = widget;
      m_widget->installEventFilter(this);
      m_widget->setFocusProxy(this);
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
      //qDebug() << "f: " << focusWidget();
      //qDebug() << ""
      if(event->type() == QEvent::KeyPress) {
        //qDebug() << "k fo: " << focusWidget();
      }
      if(watched == m_widget) {
        if(event->type() == QEvent::Resize) {
          resize(m_widget->width() + 2, m_widget->height() + 2);
        }
      } else if(watched == parent()) {
        if(event->type() == QEvent::Move) {
          move_to_parent();
        } else if(event->type() == QEvent::FocusOut &&
            focusWidget() != nullptr &&
            !isAncestorOf(focusWidget())) {
          auto a = focusWidget();
          hide();
        } else if(event->type() == QEvent::Wheel) {
          hide();
        } else if(event->type() == QEvent::KeyPress) {
          auto e = static_cast<QKeyEvent*>(event);
          if(e->key() == Qt::Key_Escape) {
            hide();
          } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            qDebug() << e;
            if(isVisible()) {
              hide();
            } else {
              move_to_parent();
              show();
            }
            return true;
          }
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
      painter.setPen(QColor("#C8C8C8"));
      painter.drawRect(0, 0, width() - 1, height() - 1);
      painter.setPen("#4B23A0");
      painter.drawLine(0, 0, static_cast<QWidget*>(parent())->width(), 0);
    }

  private:
    QWidget* m_widget;
    DropShadow* m_shadow;

    void move_to_parent() {
      auto parent_widget = static_cast<QWidget*>(parent());
      auto pos = parent_widget->mapToGlobal(parent_widget->pos());
      move(100, 100);
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
      auto font = QFont("Roboto");
      font.setPixelSize(scale_height(12));
      setFont(font);
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
      // TODO: move to anon namespace
      auto LEFT_PADDING = []{ return scale_width(8); };
      auto RIGHT_PADDING = []{ return scale_width(12); };
      auto metrics = QFontMetrics(font());
      auto shortened_text = metrics.elidedText(
        m_item_delegate.displayText(m_value), Qt::ElideRight,
        width() - RIGHT_PADDING());
      painter.drawText(LEFT_PADDING(), metrics.height(), shortened_text);
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
      //setFocusProxy(parent);
      m_scroll_area = new ScrollArea(this);
      m_scroll_area->setWidgetResizable(true);
      auto main_widget = new QWidget(this);
      m_layout = new QVBoxLayout(main_widget);
      m_layout->setContentsMargins({});
      m_layout->setSpacing(0);
      m_scroll_area->setWidget(main_widget);
      set_widget(m_scroll_area);
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
            return true;
          } else if(e->key() == Qt::Key_Down) {
            qDebug() << watched;
            qDebug() << "down";
            focus_next();
            return true;
          } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            qDebug() << "input enter key";
            if(isVisible() && m_highlight_index) {
              qDebug() << "valid input submitted";
            }
          }
        }
      }
      return DropDownWindow::eventFilter(watched, event);
    }
    void focusInEvent(QFocusEvent* event) override {
      auto a = 0;
    }
    void hideEvent(QHideEvent* event) override {
      m_scroll_area->verticalScrollBar()->setValue(0);
      if(m_highlight_index) {
        get_widget(*m_highlight_index)->reset_highlight();
        m_highlight_index = boost::none;
      }
    }
    void keyPressEvent(QKeyEvent* event) override {
      if(event->key() == Qt::Key_Up) {
        focus_previous();
      } else if(event->key() == Qt::Key_Down) {
        focus_next();
      } else if(event->key() == Qt::Key_Escape) {
        hide();
      } else if(event->key() == Qt::Key_Enter) {
        qDebug() << "list enter key";
      }
      DropDownWindow::keyPressEvent(event);
    }

  private:
    int m_max_displayed_items;
    QVBoxLayout* m_layout;
    ScrollArea* m_scroll_area;
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
        auto highlighted_widget = get_widget(*m_highlight_index);
        highlighted_widget->set_highlight();
        m_scroll_area->ensureWidgetVisible(highlighted_widget, 0, 0);
        return;
      }
      get_widget(*m_highlight_index)->reset_highlight();
      m_highlight_index = (++(*m_highlight_index)) % m_layout->count();
      auto highlighted_widget = get_widget(*m_highlight_index);
      highlighted_widget->set_highlight();
      m_scroll_area->ensureWidgetVisible(highlighted_widget, 0, 0);
    }
    void focus_previous() {
      if(m_layout->count() == 0) {
        return;
      }
      if(!m_highlight_index) {
        m_highlight_index = m_layout->count() - 1;
        auto highlighted_widget = get_widget(*m_highlight_index);
        highlighted_widget->set_highlight();
        m_scroll_area->ensureWidgetVisible(highlighted_widget, 0, 0);
        return;
      }
      get_widget(*m_highlight_index)->reset_highlight();
      --(*m_highlight_index);
      if(*m_highlight_index < 0) {
        m_highlight_index = m_layout->count() - 1;
      }
      auto highlighted_widget = get_widget(*m_highlight_index);
      highlighted_widget->set_highlight();
      m_scroll_area->ensureWidgetVisible(highlighted_widget, 0, 0);
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
    void focusInEvent(QFocusEvent* event) override {
      qDebug() << "dd in";
    }
    void mousePressEvent(QMouseEvent* event) override {
      if(event->button() == Qt::LeftButton) {
        if(!m_menu_list->isVisible()) {
          qDebug() << focusWidget();
          auto a = focusWidget();
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
      if(hasFocus() || isAncestorOf(focusWidget())) {
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
  dropdown1->setFocus();
  application->exec();
}
