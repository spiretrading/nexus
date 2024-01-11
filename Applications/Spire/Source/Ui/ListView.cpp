#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FocusObserver.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;

  auto reverse(QBoxLayout::Direction direction) {
    if(direction == QBoxLayout::TopToBottom) {
      return QBoxLayout::LeftToRight;
    }
    return QBoxLayout::TopToBottom;
  }

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(ListItemGap(scale_height(DEFAULT_GAP))).
      set(ListOverflowGap(scale_width(DEFAULT_OVERFLOW_GAP)));
    return style;
  }

  struct QWidgetItemView : ListCurrentController::ItemView {
    QWidget* m_widget;

    QWidgetItemView(QWidget& widget)
      : m_widget(&widget) {}

    bool is_selectable() const override {
      return m_widget->isEnabled();
    }

    QRect get_geometry() const override {
      return m_widget->frameGeometry();
    }
  };

  bool test_visibility(const QWidget& container, const QRect& geometry) {
    auto widget_geometry =
      QRect(container.mapToGlobal(geometry.topLeft()), geometry.size());
    auto parent_local_rect = container.parentWidget()->rect();
    auto parent_geometry = QRect(
      container.parentWidget()->mapToGlobal(parent_local_rect.topLeft()),
      parent_local_rect.size());
    return !widget_geometry.intersected(parent_geometry).isEmpty();
  }
}

ListView::ItemEntry::ItemEntry(int index)
  : m_item(new ListItem()),
    m_index(index),
    m_click_observer(*m_item) {}

QWidget* ListView::default_view_builder(
    const std::shared_ptr<AnyListModel>& list, int index) {
  return make_label(to_text(list->get(index)));
}

ListView::ListView(std::shared_ptr<AnyListModel> list, QWidget* parent)
  : ListView(std::move(list), default_view_builder, parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  ViewBuilder<> view_builder, QWidget* parent)
  : ListView(
      std::move(list), std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<ListSingleSelectionModel>(), std::move(view_builder),
      parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<SelectionModel> selection, ViewBuilder<> view_builder,
  QWidget* parent)
  : ListView(
      std::move(list), std::make_shared<LocalValueModel<optional<int>>>(),
      std::move(selection), std::move(view_builder), parent) {}

ListView::ListView(
    std::shared_ptr<AnyListModel> list, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder<> view_builder,
    QWidget* parent)
    : QWidget(parent),
      m_list(std::move(list)),
      m_current_controller(std::move(current)),
      m_selection_controller(std::move(selection)),
      m_view_builder(std::move(view_builder)),
      m_direction(Qt::Vertical),
      m_overflow(Overflow::NONE),
      m_direction_policy(QSizePolicy::Fixed),
      m_perpendicular_policy(QSizePolicy::Expanding),
      m_item_gap(DEFAULT_GAP),
      m_overflow_gap(DEFAULT_OVERFLOW_GAP),
      m_query_timer(new QTimer(this)) {
  for(auto i = 0; i != m_list->get_size(); ++i) {
    make_item_entry(i);
  }
  auto& selection_model = m_selection_controller.get_selection();
  for(auto i = 0; i != selection_model->get_size(); ++i) {
    m_items[selection_model->get(i)]->m_item->set_selected(true);
  }
  on_current(none, m_current_controller.get_current()->get());
  setFocusPolicy(Qt::StrongFocus);
  update_focus(none);
  auto body = new QWidget();
  auto body_layout = new QBoxLayout(QBoxLayout::LeftToRight, body);
  body_layout->setContentsMargins({});
  body->installEventFilter(this);
  m_box = new Box(body);
  enclose(*this, *m_box);
  proxy_style(*this, *m_box);
  set_style(*this, DEFAULT_STYLE());
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&ListView::on_style, this));
  const auto QUERY_TIMEOUT_MS = 500;
  m_query_timer->setSingleShot(true);
  m_query_timer->setInterval(QUERY_TIMEOUT_MS);
  connect(m_query_timer, &QTimer::timeout, this,
    std::bind_front(&ListView::on_query_timer_expired, this));
  m_list_connection = m_list->connect_operation_signal(
    std::bind_front(&ListView::on_list_operation, this));
  m_current_connection = m_current_controller.connect_update_signal(
    std::bind_front(&ListView::on_current, this));
  m_selection_connection = m_selection_controller.connect_operation_signal(
    std::bind_front(&ListView::on_selection, this));
  update_layout();
}

const std::shared_ptr<AnyListModel>& ListView::get_list() const {
  return m_list;
}

const std::shared_ptr<ListView::CurrentModel>& ListView::get_current() const {
  return m_current_controller.get_current();
}

const std::shared_ptr<ListView::SelectionModel>&
    ListView::get_selection() const {
  return m_selection_controller.get_selection();
}

ListItem* ListView::get_list_item(int index) {
  if(index < 0 || index >= static_cast<int>(m_items.size())) {
    return nullptr;
  }
  return m_items[index]->m_item;
}

void ListView::set_direction_size_policy(QSizePolicy::Policy policy) {
  set_item_size_policy(policy, m_perpendicular_policy);
}

void ListView::set_perpendicular_size_policy(QSizePolicy::Policy policy) {
  set_item_size_policy(m_direction_policy, policy);
}

void ListView::set_item_size_policy(QSizePolicy::Policy direction_policy,
    QSizePolicy::Policy perpendicular_policy) {
  if(m_direction_policy == direction_policy &&
      m_perpendicular_policy == perpendicular_policy) {
    return;
  }
  m_direction_policy = direction_policy;
  m_perpendicular_policy = perpendicular_policy;
  update_layout();
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ListView::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parentWidget()) {
    if(event->type() == QEvent::Resize) {
      update_visible_region();
    }
  } else if(event->type() == QEvent::Resize) {
    update_visible_region();
  }
  return QWidget::eventFilter(watched, event);
}

bool ListView::event(QEvent* event) {
  if(event->type() == QEvent::ParentAboutToChange) {
    if(auto parent = parentWidget()) {
      parent->removeEventFilter(this);
    }
  } else if(event->type() == QEvent::ParentChange) {
    update_parent();
  }
  return QWidget::event(event);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
      m_current_controller.navigate_home();
      break;
    case Qt::Key_End:
      m_current_controller.navigate_end();
      break;
    case Qt::Key_Up:
      if(m_direction == Qt::Orientation::Vertical) {
        m_current_controller.navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_previous(m_direction);
      }
      break;
    case Qt::Key_Down:
      if(m_direction == Qt::Orientation::Vertical) {
        m_current_controller.navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_next(m_direction);
      }
      break;
    case Qt::Key_Left:
      if(m_direction == Qt::Orientation::Horizontal) {
        m_current_controller.navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_previous(m_direction);
      }
      break;
    case Qt::Key_Right:
      if(m_direction == Qt::Orientation::Horizontal) {
        m_current_controller.navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_next(m_direction);
      }
      break;
    case Qt::Key_A:
      if(event->modifiers() & Qt::Modifier::CTRL && !event->isAutoRepeat()) {
        m_selection_controller.select_all();
      } else {
        append_query(event->text());
      }
      break;
    case Qt::Key_Control:
      m_keys.insert(Qt::Key_Control);
      m_selection_controller.set_mode(
        ListSelectionController::Mode::INCREMENTAL);
      break;
    case Qt::Key_Shift:
      m_keys.insert(Qt::Key_Shift);
      m_selection_controller.set_mode(ListSelectionController::Mode::RANGE);
      break;
    default:
      {
        auto text = event->text();
        if(text.size() == 1 && (text[0].isLetterOrNumber() || text[0] == '_')) {
          append_query(text);
        } else {
          QWidget::keyPressEvent(event);
        }
      }
  }
}

void ListView::keyReleaseEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Control:
      m_keys.erase(Qt::Key_Control);
      if(m_selection_controller.get_mode() ==
          ListSelectionController::Mode::INCREMENTAL) {
        if(m_keys.count(Qt::Key_Shift) == 1) {
          m_selection_controller.set_mode(ListSelectionController::Mode::RANGE);
        } else {
          m_selection_controller.set_mode(
            ListSelectionController::Mode::SINGLE);
        }
      }
      break;
    case Qt::Key_Shift:
      m_keys.erase(Qt::Key_Shift);
      if(m_selection_controller.get_mode() ==
          ListSelectionController::Mode::RANGE) {
        if(m_keys.count(Qt::Key_Control) == 1) {
          m_selection_controller.set_mode(
            ListSelectionController::Mode::INCREMENTAL);
        } else {
          m_selection_controller.set_mode(
            ListSelectionController::Mode::SINGLE);
        }
      }
      break;
  }
}

void ListView::moveEvent(QMoveEvent* event) {
  update_visible_region();
}

void ListView::resizeEvent(QResizeEvent* event) {
  update_visible_region();
}

void ListView::showEvent(QShowEvent* event) {
  update_parent();
  update_layout();
}

void ListView::append_query(const QString& query) {
  m_query += query;
  if(!m_items.empty()) {
    auto start = m_current_controller.get_current()->get().get_value_or(-1);
    auto i = (start + 1) % static_cast<int>(m_items.size());
    auto is_repeated_query = m_query.count(m_query.at(0)) == m_query.count();
    auto short_match = optional<int>();
    while(i != start) {
      if(m_items[i]->m_item->isEnabled()) {
        auto item_text = to_text(m_list->get(i)).toLower();
        if(item_text.startsWith(m_query.toLower())) {
          short_match = none;
          m_current_controller.get_current()->set(i);
          break;
        } else if(is_repeated_query &&
            !short_match && item_text.startsWith(m_query[0])) {
          short_match = i;
        }
      }
      ++i;
      if(i == m_items.size()) {
        if(start == -1) {
          break;
        }
        i = 0;
      }
    }
    if(short_match) {
      m_current_controller.get_current()->set(*short_match);
    }
  }
  m_query_timer->start();
}

void ListView::update_focus(optional<int> current) {
  if(m_focus_index && m_focus_index != current &&
      *m_focus_index < static_cast<int>(m_items.size())) {
    m_items[*m_focus_index]->m_item->setFocusPolicy(Qt::ClickFocus);
  }
  if(current) {
    m_focus_index = *current;
  } else if(!m_items.empty()) {
    m_focus_index = 0;
  } else {
    m_focus_index = none;
  }
  if(m_focus_index) {
    auto& item = *m_items[*m_focus_index]->m_item;
    item.setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(&item);
  } else {
    setFocusProxy(nullptr);
  }
}

void ListView::make_item_entry(int index) {
  auto entry = new ItemEntry(index);
  m_items.emplace(m_items.begin() + index, entry);
  entry->m_click_connection =
    entry->m_click_observer.connect_click_signal(std::bind_front(
      &ListView::on_item_click, this, std::ref(*m_items[index])));
  entry->m_submit_connection = entry->m_item->connect_submit_signal(
    [=, item = m_items[index].get()] {
      on_item_submitted(*item);
    });
  m_current_controller.add(
    std::make_unique<QWidgetItemView>(*entry->m_item), index);
  m_selection_controller.add(index);
}

void ListView::add_item(int index) {
  make_item_entry(index);
  for(auto i = m_items.begin() + index + 1; i != m_items.end(); ++i) {
    ++(*i)->m_index;
  }
  if(m_focus_index && *m_focus_index > index) {
    ++*m_focus_index;
  }
  update_layout();
}

void ListView::remove_item(int index) {
  auto item = m_items[index]->m_item;
  item->deleteLater();
  m_items.erase(m_items.begin() + index);
  for(auto i = m_items.begin() + index; i != m_items.end(); ++i) {
    --(*i)->m_index;
  }
  if(m_focus_index) {
    if(*m_focus_index == index) {
      m_focus_index = none;
    } else if(*m_focus_index > index) {
      --*m_focus_index;
    }
  }
  auto current_blocker = shared_connection_block(m_current_connection);
  auto selection_blocker = shared_connection_block(m_selection_connection);
  m_current_controller.remove(index);
  m_selection_controller.remove(index);
  on_current(none, m_current_controller.get_current()->get());
  update_layout();
}

void ListView::move_item(int source, int destination) {
  if(source < destination) {
    for(auto i = std::next(m_items.begin(), source + 1);
        i != std::next(m_items.begin(), destination + 1); ++i) {
      --(*i)->m_index;
    }
    std::rotate(std::next(m_items.begin(), source), std::next(m_items.begin(),
      source + 1), std::next(m_items.begin(), destination + 1));
    if(m_focus_index &&
        (*m_focus_index >= source || *m_focus_index <= destination)) {
      --*m_focus_index;
    }
  } else {
    for(auto i = std::next(m_items.begin(), destination);
        i != std::next(m_items.begin(), source); ++i) {
      ++(*i)->m_index;
    }
    std::rotate(std::next(m_items.rbegin(), m_items.size() - source - 1),
      std::next(m_items.rbegin(), m_items.size() - source), std::next(
      m_items.rbegin(), m_items.size() - destination));
    if(m_focus_index &&
        (*m_focus_index >= source || *m_focus_index <= destination)) {
      ++*m_focus_index;
    }
  }
  m_items[destination]->m_index = destination;
  m_current_controller.move(source, destination);
  m_selection_controller.move(source, destination);
  update_layout();
}

void ListView::update_layout() {
  auto& body = *m_box->get_body();
  if(m_direction == Qt::Orientation::Horizontal && m_overflow ==
      Overflow::NONE || m_direction == Qt::Orientation::Vertical &&
      m_overflow == Overflow::WRAP) {
    if(body.sizePolicy().horizontalPolicy() != QSizePolicy::Preferred ||
        body.sizePolicy().verticalPolicy() != QSizePolicy::Expanding) {
      body.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
      body.updateGeometry();
    }
  } else if(body.sizePolicy().horizontalPolicy() != QSizePolicy::Expanding ||
      body.sizePolicy().verticalPolicy() != QSizePolicy::Preferred) {
    body.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    body.updateGeometry();
  }
  auto& body_layout = *static_cast<QBoxLayout*>(body.layout());
  while(auto item = body_layout.takeAt(body_layout.count() - 1)) {
    delete item;
  }
  auto [direction, alignment] = [&] {
    if(m_direction == Qt::Orientation::Horizontal) {
      return std::tuple(QBoxLayout::TopToBottom, Qt::AlignLeft);
    }
    return std::tuple(QBoxLayout::LeftToRight, Qt::AlignTop);
  }();
  body_layout.setDirection(direction);
  body_layout.setSpacing(m_overflow_gap);
  auto max_size = [&] {
    if(m_overflow == Overflow::NONE) {
      return QWIDGETSIZE_MAX;
    } else if(m_direction == Qt::Orientation::Horizontal) {
      return body.width();
    }
    return body.height();
  }();
  auto i = m_items.begin();
  while(i != m_items.end()) {
    auto remaining_size = max_size;
    auto inner_layout = new QBoxLayout(reverse(direction));
    inner_layout->setContentsMargins({});
    inner_layout->setSpacing(m_item_gap);
    inner_layout->setAlignment(alignment);
    while(i != m_items.end()) {
      auto item_size = [&] {
        if(m_direction == Qt::Orientation::Horizontal) {
          return (*i)->m_item->sizeHint().width();
        } else {
          return (*i)->m_item->sizeHint().height();
        }
      }();
      remaining_size -= item_size;
      if(remaining_size < 0 && remaining_size + item_size != max_size) {
        break;
      }
      remaining_size -= inner_layout->spacing();
      if(m_direction == Qt::Orientation::Horizontal) {
        (*i)->m_item->setSizePolicy(
          m_direction_policy, m_perpendicular_policy);
      } else {
        (*i)->m_item->setSizePolicy(
          m_perpendicular_policy, m_direction_policy);
      }
      inner_layout->addWidget((*i)->m_item);
      ++i;
    }
    body_layout.addLayout(inner_layout);
  }
  update_visible_region();
}

void ListView::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  update_visible_region();
}

void ListView::update_visible_region() {
  if(!parentWidget()) {
    return;
  }
  auto position = 0;
  for(auto& item : m_items) {
    if(item == m_items.front()) {
      if(!item->m_item->is_mounted()) {
        item->m_item->mount(*m_view_builder(m_list, item->m_index));
      }
    } else {
      auto geometry = QRect(QPoint(0, position), item->m_item->size());
      if(test_visibility(*this, geometry)) {
        if(!item->m_item->is_mounted()) {
          item->m_item->mount(*m_view_builder(m_list, item->m_index));
        }
      } else {
        if(item->m_item->is_mounted()) {
          item->m_item->unmount();
        } else if(item->m_item->sizeHint().isEmpty()) {
          auto size = m_items.front()->m_item->sizeHint();
          auto size_policy = m_items.front()->m_item->sizePolicy();
          item->m_item->mount(*new QSpacerItem(size.width(), size.height(),
            size_policy.horizontalPolicy(), size_policy.verticalPolicy()));
        }
      }
    }
    position += item->m_item->size().height() + m_item_gap;
  }
}

void ListView::on_item_click(ItemEntry& item) {
  m_selection_controller.click(item.m_index);
}

void ListView::on_list_operation(const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      add_item(operation.m_index);
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      remove_item(operation.m_index);
    },
    [&] (const AnyListModel::MoveOperation& operation) {
      move_item(operation.m_source, operation.m_destination);
    });
}

void ListView::on_current(optional<int> previous, optional<int> current) {
  update_focus(current);
  if(previous && previous != current) {
    m_items[*previous]->m_item->set_current(false);
  }
  if(find_focus_state(*this) != FocusObserver::State::NONE) {
    if(current) {
      m_items[*current]->m_item->setFocus();
    } else {
      setFocus();
    }
  }
  if(current) {
    auto& item = *m_items[*current];
    item.m_item->set_current(true);
    m_selection_controller.navigate(*current);
  }
}

void ListView::on_selection(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      m_items[operation.get_value()]->m_item->set_selected(true);
    },
    [&] (const ListModel<int>::RemoveOperation& operation) {
      m_items[operation.get_value()]->m_item->set_selected(false);
    },
    [&] (const ListModel<int>::UpdateOperation& operation) {
      m_items[operation.get_previous()]->m_item->set_selected(false);
      m_items[operation.get_value()]->m_item->set_selected(true);
    });
}

void ListView::on_item_submitted(ItemEntry& item) {
  m_current_controller.get_current()->set(item.m_index);
  m_submit_signal(m_list->get(item.m_index));
}

void ListView::on_style() {
  auto& stylist = find_stylist(*this);
  auto has_update = std::make_shared<bool>(false);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const ListItemGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_item_gap = gap;
          *has_update = true;
        });
      },
      [&] (const ListOverflowGap& gap) {
        stylist.evaluate(gap, [=] (auto gap) {
          m_overflow_gap = gap;
          *has_update = true;
        });
      },
      [&] (EnumProperty<Qt::Orientation> direction) {
        stylist.evaluate(direction, [=] (auto direction) {
          m_direction = direction;
          *has_update = true;
        });
      },
      [&] (EnumProperty<EdgeNavigation> edge_navigation) {
        stylist.evaluate(edge_navigation, [=] (auto edge_navigation) {
          m_current_controller.set_edge_navigation(edge_navigation);
          *has_update = true;
        });
      },
      [&] (EnumProperty<Overflow> overflow) {
        stylist.evaluate(overflow, [=] (auto overflow) {
          m_overflow = overflow;
          *has_update = true;
        });
      });
  }
  if(*has_update) {
    update_layout();
  }
}

void ListView::on_query_timer_expired() {
  m_query.clear();
}
