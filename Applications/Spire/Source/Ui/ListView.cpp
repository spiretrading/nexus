#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
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
}

ListView::ItemEntry::ItemEntry(ListItem& item, int index)
  : m_item(&item),
    m_index(index),
    m_is_current(false),
    m_click_observer(*m_item) {}

void ListView::ItemEntry::set(bool is_current) {
  if(m_is_current == is_current) {
    return;
  }
  m_is_current = is_current;
  if(m_is_current) {
    match(*m_item, Current());
  } else {
    unmatch(*m_item, Current());
  }
}

QWidget* ListView::default_view_builder(
    const std::shared_ptr<AnyListModel>& list, int index) {
  return make_label(displayText(list->get(index)));
}

ListView::ListView(std::shared_ptr<AnyListModel> list, QWidget* parent)
  : ListView(std::move(list), default_view_builder, parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  ViewBuilder<> view_builder, QWidget* parent)
  : ListView(
      std::move(list), std::make_shared<LocalValueModel<optional<int>>>(),
      std::make_shared<SingleSelectionModel>(), std::move(view_builder),
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
      m_current(std::move(current)),
      m_last_current(m_current->get()),
      m_selection_controller(std::move(selection)),
      m_view_builder(std::move(view_builder)),
      m_direction(Qt::Vertical),
      m_edge_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_direction_policy(QSizePolicy::Fixed),
      m_perpendicular_policy(QSizePolicy::Expanding),
      m_item_gap(DEFAULT_GAP),
      m_overflow_gap(DEFAULT_OVERFLOW_GAP),
      m_query_timer(new QTimer(this)),
      m_focus_reason(Qt::OtherFocusReason) {
  setFocusPolicy(Qt::StrongFocus);
  for(auto i = 0; i != m_list->get_size(); ++i) {
    make_item_entry(i);
  }
  auto& selection_model = m_selection_controller.get_selection();
  for(auto i = 0; i != selection_model->get_size(); ++i) {
    m_items[selection_model->get(i)]->m_item->set_selected(true);
  }
  update_focus(m_last_current);
  auto body = new QWidget();
  auto body_layout = new QBoxLayout(QBoxLayout::LeftToRight, body);
  body_layout->setContentsMargins({});
  body->installEventFilter(this);
  m_box = new Box(body);
  enclose(*this, *m_box);
  set_style(*this, DEFAULT_STYLE());
  update_layout();
  proxy_style(*this, *m_box);
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  const auto QUERY_TIMEOUT_MS = 500;
  m_query_timer->setSingleShot(true);
  m_query_timer->setInterval(QUERY_TIMEOUT_MS);
  connect(
    m_query_timer, &QTimer::timeout, this, [=] { on_query_timer_expired(); });
  m_list_connection = m_list->connect_operation_signal(
    [=] (const auto& operation) { on_list_operation(operation); });
  m_current_connection = m_current->connect_update_signal(
    [=] (const auto& current) { on_current(current); });
  m_selection_controller.connect_operation_signal(
    std::bind_front(&ListView::on_selection, this));
}

const std::shared_ptr<AnyListModel>& ListView::get_list() const {
  return m_list;
}

const std::shared_ptr<ListView::CurrentModel>& ListView::get_current() const {
  return m_current;
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
  if(event->type() == QEvent::Resize) {
    update_layout();
  }
  return QWidget::eventFilter(watched, event);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
    case Qt::Key_PageUp:
      navigate_home();
      break;
    case Qt::Key_End:
    case Qt::Key_PageDown:
      navigate_end();
      break;
    case Qt::Key_Up:
      if(m_direction == Qt::Orientation::Vertical) {
        navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        cross_previous();
      }
      break;
    case Qt::Key_Down:
      if(m_direction == Qt::Orientation::Vertical) {
        navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        cross_next();
      }
      break;
    case Qt::Key_Left:
      if(m_direction == Qt::Orientation::Horizontal) {
        navigate_previous();
      } else if(m_overflow == Overflow::WRAP) {
        cross_previous();
      }
      break;
    case Qt::Key_Right:
      if(m_direction == Qt::Orientation::Horizontal) {
        navigate_next();
      } else if(m_overflow == Overflow::WRAP) {
        cross_next();
      }
      break;
    case Qt::Key_Control:
      m_selection_controller.set_mode(
        ListSelectionController::Mode::INCREMENTAL);
      break;
    case Qt::Key_Shift:
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

void ListView::append_query(const QString& query) {
  m_query += query;
  if(!m_items.empty()) {
    auto start = m_current->get().get_value_or(-1);
    auto i = (start + 1) % static_cast<int>(m_items.size());
    auto is_repeated_query = m_query.count(m_query.at(0)) == m_query.count();
    auto short_match = optional<int>();
    while(i != start) {
      if(m_items[i]->m_item->isEnabled()) {
        auto item_text = displayText(m_list->get(i)).toLower();
        if(item_text.startsWith(m_query.toLower())) {
          short_match = none;
          set(i);
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
      set(*short_match);
    }
  }
  m_query_timer->start();
}

void ListView::navigate_home() {
  navigate(1, -1, EdgeNavigation::CONTAIN);
}

void ListView::navigate_end() {
  navigate(-1, static_cast<int>(m_items.size()), EdgeNavigation::CONTAIN);
}

void ListView::navigate_next() {
  navigate(1, m_current->get().value_or(-1), m_edge_navigation);
}

void ListView::navigate_previous() {
  if(m_current->get()) {
    navigate(-1, *m_current->get(), m_edge_navigation);
  } else {
    navigate_next();
  }
}

void ListView::navigate(
    int direction, int start, EdgeNavigation edge_navigation) {
  if(m_items.empty()) {
    return;
  }
  auto i = start;
  do {
    i += direction;
    if(i < 0 || i >= static_cast<int>(m_items.size())) {
      if(edge_navigation == EdgeNavigation::CONTAIN) {
        return;
      } else if(direction == -1) {
        i = static_cast<int>(m_items.size()) - 1;
      } else {
        if(start == -1) {
          return;
        }
        i = 0;
      }
    }
  } while(i != start && !m_items[i]->m_item->isEnabled());
  set(i);
}

void ListView::cross_previous() {
  cross(-1);
}

void ListView::cross_next() {
  cross(1);
}

void ListView::cross(int direction) {
  if(m_items.empty()) {
    return;
  }
  if(!m_current->get()) {
    navigate_next();
    return;
  }
  if(m_navigation_box.isNull()) {
    if(m_current->get()) {
      m_navigation_box =
        m_items[*m_current->get()]->m_item->frameGeometry();
    } else {
      m_navigation_box = m_items.front()->m_item->frameGeometry();
    }
  }
  auto i = *m_current->get() + direction;
  auto navigation_box = m_navigation_box;
  auto candidate = -1;
  while(i >= 0 && i != static_cast<int>(m_items.size())) {
    if(m_items[i]->m_item->isEnabled()) {
      if(candidate == -1) {
        if(m_direction == Qt::Orientation::Horizontal) {
          navigation_box.moveTop(m_items[i]->m_item->pos().y());
        } else {
          navigation_box.moveLeft(m_items[i]->m_item->pos().x());
        }
      }
      if((m_direction == Qt::Orientation::Horizontal &&
          direction * m_navigation_box.y() < direction * navigation_box.y() ||
          m_direction == Qt::Orientation::Vertical &&
          direction * m_navigation_box.x() < direction * navigation_box.x()) &&
          navigation_box.intersects(m_items[i]->m_item->frameGeometry())) {
        candidate = i;
        if(direction == 1) {
          break;
        }
      } else if(candidate != -1) {
        break;
      }
    }
    i += direction;
  }
  if(candidate == -1) {
    return;
  }
  m_navigation_box = navigation_box;
  set(candidate);
  if(candidate == m_current->get()) {
    m_navigation_box = navigation_box;
  }
}

void ListView::set(optional<int> current) {
  if(!current && !m_current->get() || m_items.empty()) {
    return;
  }
  if(auto& previous_index = m_current->get();
      previous_index && *previous_index < static_cast<int>(m_items.size())) {
    auto& previous_item = *m_items[*previous_index];
    if(previous_index == current && previous_item.m_is_current) {
      return;
    }
    previous_item.set(false);
  }
  if(current) {
    if(*current >= static_cast<int>(m_items.size())) {
      current = static_cast<int>(m_items.size()) - 1;
    }
    m_items[*current]->set(true);
  }
  m_last_current = current;
  m_focus_reason = Qt::ShortcutFocusReason;
  m_current->set(current);
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
  auto item = new ListItem(*m_view_builder(m_list, index));
  m_items.emplace(m_items.begin() + index, new ItemEntry(*item, index));
  m_items[index]->m_click_connection =
    m_items[index]->m_click_observer.connect_click_signal(std::bind_front(
      &ListView::on_item_click, this, std::ref(*m_items[index])));
  m_items[index]->m_submit_connection = item->connect_submit_signal(
    [=, item = m_items[index].get()] {
      on_item_submitted(*item);
    });
}

void ListView::add_item(int index) {
  make_item_entry(index);
  for(auto i = m_items.begin() + index + 1; i != m_items.end(); ++i) {
    ++(*i)->m_index;
  }
  update_layout();
  if(m_current->get() && *m_current->get() >= index) {
    set(*m_current->get() + 1);
  }
  m_selection_controller.add(index);
}

void ListView::remove_item(int index) {
  auto item = m_items[index]->m_item;
  item->deleteLater();
  m_items.erase(m_items.begin() + index);
  for(auto i = m_items.begin() + index; i != m_items.end(); ++i) {
    --(*i)->m_index;
  }
  m_selection_controller.remove(index);
  if(m_current->get()) {
    if(m_current->get() == index) {
      set(*m_current->get());
    } else if(m_current->get() > index) {
      set(*m_current->get() - 1);
    }
  }
  update_layout();
}

void ListView::move_item(int source, int destination) {
  auto direction = [&] {
    if(source < destination) {
      for(auto i = std::next(m_items.begin(), source + 1);
          i != std::next(m_items.begin(), destination + 1); ++i) {
        --(*i)->m_index;
      }
      std::rotate(std::next(m_items.begin(), source), std::next(m_items.begin(),
        source + 1), std::next(m_items.begin(), destination + 1));
      return -1;
    } else {
      for(auto i = std::next(m_items.begin(), destination);
          i != std::next(m_items.begin(), source); ++i) {
        ++(*i)->m_index;
      }
      std::rotate(std::next(m_items.rbegin(), m_items.size() - source - 1),
        std::next(m_items.rbegin(), m_items.size() - source), std::next(
        m_items.rbegin(), m_items.size() - destination));
      return 1;
    }
  }();
  m_items[destination]->m_index = destination;
  auto adjust = [&] (auto& value) {
    if(value && (*value >= source || *value <= destination)) {
      *value += direction;
      return true;
    }
    return false;
  };
  adjust(m_last_current);
  adjust(m_focus_index);
  auto current = m_current->get();
  if(adjust(current)) {
    m_current->set(current);
  }
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
        (*i)->m_item->setSizePolicy(m_direction_policy, m_perpendicular_policy);
      } else {
        (*i)->m_item->setSizePolicy(m_perpendicular_policy, m_direction_policy);
      }
      inner_layout->addWidget((*i)->m_item);
      ++i;
    }
    body_layout.addLayout(inner_layout);
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

void ListView::on_current(const optional<int>& current) {
  update_focus(current);
  if(m_last_current && m_last_current != current) {
    m_items[*m_last_current]->set(false);
  }
  if(current && (hasFocus() || isAncestorOf(focusWidget()))) {
    m_items[*current]->m_item->setFocus(m_focus_reason);
  } else if(isAncestorOf(focusWidget())) {
    setFocus();
  }
  m_focus_reason = Qt::OtherFocusReason;
  if(current) {
    auto& item = *m_items[*current];
    m_navigation_box = item.m_item->frameGeometry();
    item.set(true);
  } else {
    m_navigation_box = QRect();
  }
  m_last_current = current;
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
  m_navigation_box = item.m_item->frameGeometry();
  set(item.m_index);
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
          m_edge_navigation = edge_navigation;
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
  if(has_update) {
    update_layout();
  }
}

void ListView::on_query_timer_expired() {
  m_query.clear();
}
