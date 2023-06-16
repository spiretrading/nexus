#include "Spire/Ui/AdaptiveBox.hpp"
#include <queue>
#include <ranges>
#include <unordered_set>
#include <QResizeEvent>
#include "Spire/Spire/ExtensionCache.hpp"
#include "Spire/Ui/Layouts.hpp"

using namespace Spire;

namespace {
  class PlaceholderLayoutItem;

  struct SharedWidget {
    const QWidget* m_widget;
    PlaceholderLayoutItem* m_controlling_placeholder;
    std::unordered_set<PlaceholderLayoutItem*> m_items;

    SharedWidget(const QWidget& widget)
      : m_widget(&widget),
        m_controlling_placeholder(nullptr) {}

    void register_placeholder(PlaceholderLayoutItem& placeholder) {
      m_items.insert(&placeholder);
    }

    void release_placeholder(PlaceholderLayoutItem& placeholder) {
      m_items.erase(&placeholder);
      if(m_items.empty()) {
        delete m_widget;
      }
    }

    bool take_control(PlaceholderLayoutItem* placeholder) {
      if(m_controlling_placeholder || !m_items.contains(placeholder)) {
        return false;
      }
      m_controlling_placeholder = placeholder;
      return true;
    }

    bool release_control(PlaceholderLayoutItem* placeholder) {
      if(m_controlling_placeholder != placeholder) {
        return false;
      }
      m_controlling_placeholder = nullptr;
      return true;
    }
  };

  struct WidgetItem : QWidgetItem {
    WidgetItem(QWidget& widget)
      : QWidgetItem(&widget) {}

    bool isEmpty() const override {
      return false;
    }
  };

  class PlaceholderLayoutItem : public QLayoutItem {
    public:
      PlaceholderLayoutItem(QWidget& widget)
          : m_item(widget),
            m_filter(std::make_unique<EventFilter>(*this)),
            m_shared_widget(find_extension<SharedWidget>(widget)),
            m_is_visible(false),
            m_parent(nullptr) {
        m_shared_widget->register_placeholder(*this);
        m_item.widget()->setParent(nullptr);
      }

      ~PlaceholderLayoutItem() {
        m_shared_widget->release_placeholder(*this);
      }

      void set_parent(QWidget& parent) {
        if(m_parent) {
          m_parent->removeEventFilter(m_filter.get());
        }
        m_parent = &parent;
        m_parent->installEventFilter(m_filter.get());
      }

      bool is_visible() const {
        return m_is_visible;
      }

      void set_visible(bool visible) {
        m_is_visible = visible;
        take_control();
      }

      QSize sizeHint() const {
        return m_item.sizeHint();
      }

      QSize minimumSize() const {
        return m_item.minimumSize();
      }

      QSize maximumSize() const {
        return m_item.maximumSize();
      }

      Qt::Orientations expandingDirections() const {
        return m_item.expandingDirections();
      }

      bool isEmpty() const override {
        return false;
      }

      void setGeometry(const QRect& rect) override {
        m_item.setGeometry(rect);
      }

      QRect geometry() const override{
        return m_item.geometry();
      }

      bool hasHeightForWidth() const override {
        return m_item.hasHeightForWidth();
      }

      int heightForWidth(int w) const override {
        return m_item.heightForWidth(w);
      }

    private:
      struct EventFilter : QObject {
        PlaceholderLayoutItem* m_item;

        EventFilter(PlaceholderLayoutItem& item)
          : m_item(&item) {}

        bool eventFilter(QObject* watched, QEvent* event) override {
          if(event->type() == QEvent::Show ||
              event->type() == QEvent::ShowToParent) {
            m_item->set_visible(true);
          } else if(event->type() == QEvent::Hide ||
              event->type() == QEvent::HideToParent) {
            m_item->set_visible(false);
          }
          return QObject::eventFilter(watched, event);
        }
      };

      WidgetItem m_item;
      std::unique_ptr<EventFilter> m_filter;
      std::shared_ptr<SharedWidget> m_shared_widget;
      bool m_is_visible;
      QWidget* m_parent;

      void take_control() {
        if(is_visible()) {
          if(m_shared_widget->take_control(this)) {
            m_item.widget()->setParent(m_parent);
            m_item.widget()->show();
          }
        } else if(m_shared_widget->release_control(this)) {
          m_item.widget()->setParent(nullptr);
        }
      }
  };

  struct GridLayoutItem {
    QLayoutItem* m_item;
    int m_row;
    int m_column;
    int m_row_span;
    int m_column_span;
  };

  struct BoxLayoutItem {
    QLayoutItem* m_item;
    int m_stretch;
  };

  struct LayoutItem {
    QLayoutItem* m_item;
  };

  auto make_placeholder(QWidget& widget, QWidget& parent,
      const QLayoutItem& item) {
    auto placeholder = new PlaceholderLayoutItem(widget);
    placeholder->set_parent(parent);
    placeholder->setAlignment(item.alignment());
    return placeholder;
  }

  template<typename L, typename T>
  void replace(L& layout, QWidget& parent,
      std::function<T(L& layout)> remove,
      std::function<void(L& layout, T& item, QWidget& widget)> add_widget,
      std::function<void(L& layout, T& item, QLayout& child_layout)> add_layout,
      std::function<void(L& layout, T& item)> add_others) {
    auto items = std::queue<T>();
    while(layout.count() > 0) {
      items.push(remove(layout));
    }
    while(!items.empty()) {
      auto& item = items.front();
      items.pop();
      if(auto widget = item.m_item->widget()) {
        add_widget(layout, item, *widget);
      } else if(auto child_layout = item.m_item->layout()) {
        add_layout(layout, item, *child_layout);
      } else {
        add_others(layout, item);
      }
    }
  }

  void replace_widget_with_placeholder(QLayout& layout, QWidget& parent) {
    if(auto grid_layout = dynamic_cast<QGridLayout*>(&layout)) {
      replace<QGridLayout, GridLayoutItem>(*grid_layout, parent,
        [] (QGridLayout& layout) {
          auto item = GridLayoutItem();
          layout.getItemPosition(0, &item.m_row, &item.m_column,
            &item.m_row_span, &item.m_column_span);
          item.m_item = layout.takeAt(0);
          return item;
        },
        [&] (QGridLayout& layout, GridLayoutItem& item, QWidget& widget) {
          layout.addItem(make_placeholder(widget, parent, *item.m_item),
            item.m_row, item.m_column, item.m_row_span, item.m_column_span);
          delete item.m_item;
        },
        [&] (QGridLayout& layout, GridLayoutItem& item, QLayout& child_layout) {
          replace_widget_with_placeholder(child_layout, parent);
          layout.addItem(item.m_item, item.m_row, item.m_column,
            item.m_row_span, item.m_column_span);
        },
        [] (QGridLayout& layout, GridLayoutItem& item) {
          layout.addItem(item.m_item, item.m_row, item.m_column,
            item.m_row_span, item.m_column_span);
        });
    } else if(auto box_layout = dynamic_cast<QBoxLayout*>(&layout)) {
      replace<QBoxLayout, BoxLayoutItem>(*box_layout, parent,
        [] (QBoxLayout& layout) {
          auto item = BoxLayoutItem();
          item.m_stretch = layout.stretch(0);
          item.m_item = layout.takeAt(0);
          return item;
        },
        [&] (QBoxLayout& layout, BoxLayoutItem& item, QWidget& widget) {
          layout.addItem(make_placeholder(widget, parent, *item.m_item));
          layout.setStretch(layout.count() - 1, item.m_stretch);
          delete item.m_item;
        },
        [&] (QBoxLayout& layout, BoxLayoutItem& item, QLayout& child_layout) {
          replace_widget_with_placeholder(child_layout, parent);
          layout.addItem(item.m_item);
          layout.setStretch(layout.count() - 1, item.m_stretch);
        },
        [] (QBoxLayout& layout, BoxLayoutItem& item) {
          layout.addItem(item.m_item);
          layout.setStretch(layout.count() - 1, item.m_stretch);
        });
    } else {
      replace<QLayout, LayoutItem>(layout, parent,
        [] (QLayout& layout) {
          auto item = LayoutItem();
          item.m_item = layout.takeAt(0);
          return item;
        },
        [&] (QLayout& layout, LayoutItem& item, QWidget& widget) {
          layout.addItem(make_placeholder(widget, parent, *item.m_item));
          delete item.m_item;
        },
        [&] (QLayout& layout, LayoutItem& item, QLayout& child_layout) {
          replace_widget_with_placeholder(child_layout, parent);
          layout.addItem(item.m_item);
        },
        [] (QLayout& layout, LayoutItem& item) {
          layout.addItem(item.m_item);
        });
    }
  }

  void invalidate_layout(QLayout& layout) {
    layout.invalidate();
    for(auto i = 0; i < layout.count(); ++i) {
      auto item = layout.itemAt(i);
      if(auto child_layout = item->layout()) {
        invalidate_layout(*child_layout);
      }
    }
  }

  auto get_min_abs_value_index(const std::vector<std::tuple<bool, int>>& v) {
    auto candidates = v | std::views::filter([] (auto& value) {
      return !get<0>(value);
    });
    if(std::ranges::empty(candidates)) {
      return std::distance(v.begin(), std::min_element(v.begin(), v.end(),
        [] (auto& value1, auto& value2) {
          return std::abs(get<1>(value1)) < std::abs(get<1>(value2));
        }));
    }
    return std::distance(v.begin(), std::find(v.begin(), v.end(),
      *std::ranges::min_element(candidates.begin(), candidates.end(),
        [] (auto& value1, auto& value2) {
          return get<1>(value1) < get<1>(value2);
        })));
  }
}

AdaptiveBox::AdaptiveBox(QWidget* parent)
    : QWidget(parent) {
  m_stacked_layout = new QStackedLayout(this);
  if(parent) {
    parent->installEventFilter(this);
  }
  installEventFilter(this);
}

void AdaptiveBox::add(QLayout& layout) {
  auto widget = new QWidget();
  replace_widget_with_placeholder(layout, *widget);
  widget->setLayout(&layout);
  widget->installEventFilter(this);
  m_stacked_layout->addWidget(widget);
  update_layout();
  m_stacked_layout->currentWidget()->hide();
  m_stacked_layout->currentWidget()->show();
}

QSize AdaptiveBox::sizeHint() const {
  if(auto current = m_stacked_layout->currentWidget()) {
    return current->sizeHint();
  }
  return QSize(0, 0);
}

QSize AdaptiveBox::minimumSizeHint() const {
  if(auto current = m_stacked_layout->currentWidget()) {
    return current->minimumSizeHint();
  }
  return QSize(0, 0);
}

bool AdaptiveBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == this) {
    if(event->type() == QEvent::ParentChange) {
      parentWidget()->installEventFilter(this);
    } else if(event->type() == QEvent::ParentAboutToChange) {
      parentWidget()->removeEventFilter(this);
    }
  } else if(watched == parentWidget() &&
      event->type() == QEvent::LayoutRequest) {
    update_layout();
  } else if(event->type() == QEvent::LayoutRequest) {
    if(watched == m_stacked_layout->currentWidget()) {
      auto index = m_stacked_layout->currentIndex();
      for(auto i = 0; i < m_stacked_layout->count(); ++i) {
        if(i != index) {
          invalidate_layout(*m_stacked_layout->widget(i)->layout());
        }
      }
    }
    update_layout();
  }
  return QWidget::eventFilter(watched, event);
}

void AdaptiveBox::resizeEvent(QResizeEvent* event) {
  if(event->size().isNull()) {
    return;
  }
  update_layout();
}

void AdaptiveBox::update_layout() {
  auto size_policy = sizePolicy();
  auto is_horizontal_driven = [&] {
    if(size_policy.horizontalPolicy() == QSizePolicy::Fixed ||
        size_policy.horizontalPolicy() == QSizePolicy::Preferred) {
      return true;
    }
    return false;
  }();
  auto is_vertical_driven = [&] {
    if(size_policy.verticalPolicy() == QSizePolicy::Fixed ||
        size_policy.verticalPolicy() == QSizePolicy::Preferred) {
      return true;
    }
    return false;
  }();
  auto overflows =
    std::vector<std::tuple<bool, int>>(m_stacked_layout->count());
  for(auto i = 0; i < m_stacked_layout->count(); ++i) {
    auto size_hint = m_stacked_layout->widget(i)->layout()->sizeHint();
    auto width = [&] {
      if(is_horizontal_driven) {
        return size_hint.width();
      }
      return size().width();
    }();
    auto height = [&] {
      if(is_vertical_driven) {
        return size_hint.height();
      }
      return size().height();
    }();
    if(size_hint.width() > width || size_hint.height() > height) {
      std::get<0>(overflows[i]) = true;
    } else {
      std::get<0>(overflows[i]) = false;
    }
    std::get<1>(overflows[i]) =
      width * height - size_hint.width() * size_hint.height();
  }
  m_stacked_layout->setCurrentIndex(get_min_abs_value_index(overflows));
}
