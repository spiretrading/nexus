#include "Spire/Ui/ListView.hpp"
#include <ranges>
#include <boost/signals2/shared_connection_block.hpp>
#include <QKeyEvent>
#include <QTimer>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ListCurrentIndexModel.hpp"
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
  const auto SCROLL_BUFFER = 200;

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

  int get_directed_point(QPoint point, Qt::Orientation direction) {
    if(direction == Qt::Orientation::Vertical) {
      return point.y();
    }
    return point.x();
  }

  QPoint make_directed_point(int x, int y, Qt::Orientation direction) {
    if(direction == Qt::Orientation::Vertical) {
      return QPoint(x, y);
    }
    return QPoint(y, x);
  }

  QPoint make_directed_point(int x, Qt::Orientation direction) {
    return make_directed_point(0, x, direction);
  }

  int get_directed_size(QSize size, Qt::Orientation direction) {
    if(direction == Qt::Orientation::Vertical) {
      return size.height();
    }
    return size.width();
  }

  bool test_visibility(const QWidget& container, const QRect& geometry,
      Qt::Orientation direction) {
    auto widget_geometry =
      QRect(container.mapToParent(geometry.topLeft()), geometry.size());
    if(direction == Qt::Orientation::Vertical) {
      return std::max(-SCROLL_BUFFER, widget_geometry.top()) <=
        std::min(container.parentWidget()->height() + SCROLL_BUFFER,
          widget_geometry.bottom());
    }
    return std::max(-SCROLL_BUFFER, widget_geometry.left()) <=
      std::min(container.parentWidget()->width() + SCROLL_BUFFER,
        widget_geometry.right());
  }
}

ListView::ItemEntry::ItemEntry(int index)
  : m_index(index),
    m_click_observer(m_item) {}

QWidget* ListView::default_item_builder(
    const std::shared_ptr<AnyListModel>& list, int index) {
  return make_label(to_text(list->get(index)));
}

ListView::ListView(std::shared_ptr<AnyListModel> list, QWidget* parent)
  : ListView(std::move(list), default_item_builder, parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  ListViewItemBuilder<> item_builder, QWidget* parent)
  : ListView(list, std::make_shared<ListCurrentIndexModel>(list),
      std::make_shared<ListSingleSelectionModel>(), std::move(item_builder),
      parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  ListViewItemBuilder<> item_builder, ToText to_text, QWidget* parent)
  : ListView(list, std::make_shared<ListCurrentIndexModel>(list),
      std::make_shared<ListSingleSelectionModel>(), std::move(item_builder),
      std::move(to_text), parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<SelectionModel> selection, ListViewItemBuilder<> item_builder,
  QWidget* parent)
  : ListView(list, std::make_shared<ListCurrentIndexModel>(list),
      std::move(selection), std::move(item_builder), parent) {}

ListView::ListView(std::shared_ptr<AnyListModel> list,
  std::shared_ptr<SelectionModel> selection, ListViewItemBuilder<> item_builder,
  ToText to_text, QWidget* parent)
  : ListView(list, std::make_shared<ListCurrentIndexModel>(list),
      std::move(selection), std::move(item_builder), std::move(to_text),
      parent) {}

ListView::ListView(
  std::shared_ptr<AnyListModel> list, std::shared_ptr<CurrentModel> current,
  std::shared_ptr<SelectionModel> selection,
  ListViewItemBuilder<> item_builder, QWidget* parent)
  : ListView(std::move(list), std::move(current), std::move(selection),
      std::move(item_builder), [] (const std::any& value) {
        return to_text(value);
      }, parent) {}

ListView::ListView(
    std::shared_ptr<AnyListModel> list, std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<> item_builder, ToText to_text, QWidget* parent)
    : QWidget(parent),
      m_list(std::move(list)),
      m_current_controller(std::move(current), m_list->get_size()),
      m_selection_controller(std::move(selection), m_list->get_size()),
      m_item_builder(std::move(item_builder)),
      m_to_text(std::move(to_text)),
      m_current_entry(nullptr),
      m_direction(Qt::Vertical),
      m_overflow(Overflow::NONE),
      m_visible_count(0),
      m_top_index(-1),
      m_direction_policy(QSizePolicy::Fixed),
      m_perpendicular_policy(QSizePolicy::Expanding),
      m_item_gap(DEFAULT_GAP),
      m_overflow_gap(DEFAULT_OVERFLOW_GAP),
      m_query_timer(this),
      m_initialize_count(0),
      m_is_transaction(false) {
  for(auto i : std::ranges::views::iota(0, m_list->get_size())) {
    auto& entry = make_item_entry(i);
    m_current_controller.add(
      std::make_unique<QWidgetItemView>(entry.m_item), i);
    m_selection_controller.add(i);
  }
  auto& selection_model = m_selection_controller.get_selection();
  for(auto i : std::ranges::views::iota(0, selection_model->get_size())) {
    m_items[selection_model->get(i)]->m_item.set_selected(true);
  }
  setFocusPolicy(Qt::StrongFocus);
  update_focus(none);
  auto body = new QWidget();
  auto body_layout = new QBoxLayout(QBoxLayout::LeftToRight, body);
  body_layout->setContentsMargins({});
  body->installEventFilter(this);
  m_box = new Box(body);
  enclose(*this, *m_box);
  proxy_style(*this, *m_box);
  m_style_connection =
    connect_style_signal(*this, std::bind_front(&ListView::on_style, this));
  set_style(*this, DEFAULT_STYLE());
  m_query_timer.setSingleShot(true);
  const auto QUERY_TIMEOUT_MS = 500;
  m_query_timer.setInterval(QUERY_TIMEOUT_MS);
  connect(&m_query_timer, &QTimer::timeout, this,
    std::bind_front(&ListView::on_query_timer_expired, this));
  m_list_connection = m_list->connect_operation_signal(
    std::bind_front(&ListView::on_list_operation, this));
  m_current_connection = m_current_controller.connect_update_signal(
    std::bind_front(&ListView::on_current, this));
  m_selection_connection = m_selection_controller.connect_operation_signal(
    std::bind_front(&ListView::on_selection, this));
  on_current(m_current_controller.get_current()->get());
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
  return &m_items[index]->m_item;
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
    update_layout();
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
      select_current();
      break;
    case Qt::Key_End:
      m_current_controller.navigate_end();
      select_current();
      break;
    case Qt::Key_Up:
      if(m_direction == Qt::Orientation::Vertical) {
        m_current_controller.navigate_previous();
        select_current();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_previous(m_direction);
        select_current();
      }
      break;
    case Qt::Key_Down:
      if(m_direction == Qt::Orientation::Vertical) {
        m_current_controller.navigate_next();
        select_current();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_next(m_direction);
        select_current();
      }
      break;
    case Qt::Key_Left:
      if(m_direction == Qt::Orientation::Horizontal) {
        m_current_controller.navigate_previous();
        select_current();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_previous(m_direction);
        select_current();
      }
      break;
    case Qt::Key_Right:
      if(m_direction == Qt::Orientation::Horizontal) {
        m_current_controller.navigate_next();
        select_current();
      } else if(m_overflow == Overflow::WRAP) {
        m_current_controller.cross_next(m_direction);
        select_current();
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

void ListView::showEvent(QShowEvent* event) {
  update_parent();
}

void ListView::append_query(const QString& query) {
  m_query += query;
  if(!m_items.empty()) {
    auto start = m_current_controller.get_current()->get().get_value_or(-1);
    auto i = (start + 1) % static_cast<int>(m_items.size());
    auto is_repeated_query = m_query.count(m_query.at(0)) == m_query.count();
    auto short_match = optional<int>();
    while(i != start) {
      if(m_items[i]->m_item.isEnabled()) {
        auto item_text = m_to_text(m_list->get(i)).toLower();
        if(item_text.startsWith(m_query.toLower())) {
          short_match = none;
          m_current_controller.get_current()->set(i);
          select_current();
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
      select_current();
    }
  }
  m_query_timer.start();
}

void ListView::update_focus(optional<int> current) {
  if(m_focus_index && m_focus_index != current &&
      *m_focus_index < static_cast<int>(m_items.size())) {
    m_items[*m_focus_index]->m_item.setFocusPolicy(Qt::ClickFocus);
  }
  if(current) {
    m_focus_index = *current;
  } else if(!m_items.empty()) {
    m_focus_index = 0;
  } else {
    m_focus_index = none;
  }
  if(m_focus_index) {
    auto& item = m_items[*m_focus_index]->m_item;
    item.setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(&item);
  } else {
    setFocusProxy(nullptr);
  }
}

ListView::ItemEntry& ListView::make_item_entry(int index) {
  auto entry = new ItemEntry(index);
  if(m_overflow != Overflow::NONE) {
    entry->m_item.mount(*m_item_builder.mount(m_list, entry->m_index));
  }
  m_items.emplace(m_items.begin() + index, entry);
  entry->m_click_observer->connect_click_signal(
    std::bind_front(&ListView::on_item_click, this, std::ref(*m_items[index])));
  entry->m_item.connect_submit_signal(std::bind_front(
    &ListView::on_item_submitted, this, std::ref(*m_items[index])));
  return *entry;
}

void ListView::add_item(int index) {
  auto& entry = make_item_entry(index);
  for(auto& i : m_items | std::views::drop(index + 1)) {
    ++i->m_index;
  }
  if(m_focus_index && *m_focus_index > index) {
    ++*m_focus_index;
  }
  m_current_controller.add(
    std::make_unique<QWidgetItemView>(entry.m_item), index);
  m_selection_controller.add(index);
}

void ListView::pre_remove_item(int index) {
  auto item = std::move(m_items[index]);
  m_items.erase(m_items.begin() + index);
  if(item.get() == m_current_entry) {
    m_current_entry = nullptr;
  }
  for(auto& item : m_items | std::views::drop(index)) {
    --item->m_index;
  }
  item->m_click_observer = none;
  QTimer::singleShot(0, [item = std::move(item)] () mutable {
    item = nullptr;
  });
  if(m_focus_index) {
    if(*m_focus_index == index) {
      m_focus_index = none;
    } else if(*m_focus_index > index) {
      --*m_focus_index;
    }
  }
}

void ListView::remove_item(int index) {
  m_current_controller.remove(index);
  auto blocker = shared_connection_block(m_selection_connection);
  m_selection_controller.remove(index);
}

void ListView::move_item(int source, int destination) {
  if(source < destination) {
    for(auto& i : std::ranges::subrange(std::next(m_items.begin(), source + 1),
        std::next(m_items.begin(), destination + 1))) {
      --i->m_index;
    }
    std::rotate(std::next(m_items.begin(), source), std::next(m_items.begin(),
      source + 1), std::next(m_items.begin(), destination + 1));
    if(m_focus_index &&
        (*m_focus_index >= source || *m_focus_index <= destination)) {
      --*m_focus_index;
    }
  } else {
    for(auto& i : std::ranges::subrange(std::next(m_items.begin(), destination),
        std::next(m_items.begin(), source))) {
      ++i->m_index;
    }
    std::rotate(std::next(m_items.rbegin(), m_items.size() - source - 1),
      std::next(m_items.rbegin(), m_items.size() - source),
      std::next(m_items.rbegin(), m_items.size() - destination));
    if(m_focus_index &&
        (*m_focus_index >= source || *m_focus_index <= destination)) {
      ++*m_focus_index;
    }
  }
  m_items[destination]->m_index = destination;
  m_current_controller.move(source, destination);
  m_selection_controller.move(source, destination);
}

void ListView::select_current() {
  if(auto current = m_current_controller.get_current()->get()) {
    m_selection_controller.navigate(*current);
  }
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
    body_layout.addLayout(inner_layout);
    while(i != m_items.end()) {
      auto item_size = get_directed_size((*i)->m_item.sizeHint(), m_direction);
      remaining_size -= item_size;
      if(remaining_size < 0 && remaining_size + item_size != max_size) {
        break;
      }
      remaining_size -= inner_layout->spacing();
      if(m_direction == Qt::Orientation::Horizontal) {
        (*i)->m_item.setSizePolicy(m_direction_policy, m_perpendicular_policy);
      } else {
        (*i)->m_item.setSizePolicy(m_perpendicular_policy, m_direction_policy);
      }
      auto is_linked = (*i)->m_item.parentWidget() != nullptr;
      inner_layout->addWidget(&(*i)->m_item);
      if(!is_linked) {
        link(*this, (*i)->m_item);
      }
      ++i;
    }
  }
  update_visible_region();
}

void ListView::update_parent() {
  if(auto parent = parentWidget()) {
    parent->installEventFilter(this);
  }
  initialize_visible_region();
}

void ListView::initialize_visible_region() {
  if(m_overflow != Overflow::NONE) {
    for(auto& item : m_items) {
      if(!item->m_item.is_mounted()) {
        item->m_item.mount(*m_item_builder.mount(m_list, item->m_index));
      }
    }
    return;
  }
  if(!parentWidget() || m_top_index != -1 || !isVisible()) {
    return;
  }
  ++m_initialize_count;
  QTimer::singleShot(0, this, [=] {
    --m_initialize_count;
    if(m_initialize_count != 0) {
      return;
    }
    if(!parentWidget()) {
      return;
    }
    m_visible_count = 0;
    if(m_items.empty()) {
      m_top_index = 0;
      return;
    }
    m_top_index = std::numeric_limits<int>::max();
    auto& front_item = *m_items.front();
    if(!front_item.m_item.is_mounted()) {
      front_item.m_item.mount(*m_item_builder.mount(m_list, 0));
    }
    auto top_geometry =
      QRect(QPoint(0, 0), front_item.m_item.sizeHint());
    if(test_visibility(*this, top_geometry, m_direction)) {
      m_top_index = 0;
      m_visible_count = 1;
    }
    auto position =
      get_directed_size(top_geometry.size(), m_direction) + m_item_gap;
    for(auto& item : m_items | std::views::drop(1)) {
      auto geometry = [&] {
        if(item->m_item.is_mounted()) {
          return QRect(make_directed_point(position, m_direction),
            item->m_item.sizeHint());
        }
        return QRect(make_directed_point(position, m_direction),
          front_item.m_item.sizeHint());
      }();
      auto is_visible = test_visibility(*this, geometry, m_direction);
      if(is_visible || item->m_item.is_current()) {
        if(!item->m_item.is_mounted()) {
          item->m_item.mount(*m_item_builder.mount(m_list, item->m_index));
        }
        if(is_visible) {
          m_top_index = std::min(m_top_index, item->m_index);
          ++m_visible_count;
        }
      } else if(item->m_item.is_mounted() && !item->m_item.is_current()) {
        if(auto widget = item->m_item.unmount()) {
          m_item_builder.unmount(widget, item->m_index);
        }
      } else if(item->m_item.sizeHint().isEmpty()) {
        auto size = front_item.m_item.sizeHint();
        auto size_policy = front_item.m_item.get_body().sizePolicy();
        item->m_item.mount(*new QSpacerItem(size.width(), size.height(),
          size_policy.horizontalPolicy(), size_policy.verticalPolicy()));
      }
      position += get_directed_size(geometry.size(), m_direction) + m_item_gap;
    }
  });
}

void ListView::update_visible_region() {
  if(m_top_index == -1) {
    initialize_visible_region();
    return;
  }
  if(!parentWidget() ||
      !isVisible() || m_overflow != Overflow::NONE || m_items.empty()) {
    return;
  }
  auto top_item = [&] {
    auto low = std::size_t(0);
    auto high = m_items.size() - 1;
    auto last_visible = static_cast<ItemEntry*>(nullptr);
    while(high >= low) {
      auto middle = low + (high - low) / 2;
      auto& item = m_items[middle];
      auto item_geometry = item->m_item.frameGeometry();
      if(test_visibility(*this, item_geometry, m_direction)) {
        last_visible = &*item;
        if(middle == 0) {
          break;
        }
        high = middle - 1;
      } else {
        auto widget_geometry = mapToParent(item_geometry.topLeft());
        if(get_directed_point(widget_geometry, m_direction) < 0) {
          low = middle + 1;
        } else {
          if(middle == 0) {
            break;
          }
          high = middle - 1;
        }
      }
    }
    return last_visible;
  }();
  if(top_item) {
    for(auto& item : m_items |
          std::views::drop(m_top_index) | std::views::take(m_visible_count)) {
      if(item->m_item.is_mounted() && !item->m_item.is_current() &&
          !test_visibility(*this, item->m_item.frameGeometry(), m_direction)) {
        if(auto widget = item->m_item.unmount()) {
          m_item_builder.unmount(widget, item->m_index);
        }
      }
    }
    m_top_index = top_item->m_index;
    m_visible_count = 0;
    auto position = get_directed_point(top_item->m_item.pos(), m_direction);
    for(auto& item : m_items | std::views::drop(m_top_index)) {
      auto geometry =
        QRect(make_directed_point(position, m_direction), item->m_item.size());
      if(test_visibility(*this, geometry, m_direction)) {
        if(!item->m_item.is_mounted()) {
          item->m_item.mount(*m_item_builder.mount(m_list, item->m_index));
        }
        ++m_visible_count;
      } else {
        break;
      }
      position +=
        get_directed_size(item->m_item.size(), m_direction) + m_item_gap;
    }
  }
}

void ListView::on_item_click(ItemEntry& item) {
  m_selection_controller.click(item.m_index);
}

void ListView::on_list_operation(const AnyListModel::Operation& operation) {
  m_operation_queue.Add([=] {
    visit(operation,
      [&] (AnyListModel::StartTransaction) {
        m_is_transaction = true;
      },
      [&] (AnyListModel::EndTransaction) {
        m_is_transaction = false;
      },
      [&] (const AnyListModel::AddOperation& operation) {
        add_item(operation.m_index);
      },
      [&] (const AnyListModel::PreRemoveOperation& operation) {
        pre_remove_item(operation.m_index);
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        remove_item(operation.m_index);
      },
      [&] (const AnyListModel::MoveOperation& operation) {
        move_item(operation.m_source, operation.m_destination);
      });
    if(!m_is_transaction) {
      m_top_index = -1;
      update_layout();
    }
  });
}

void ListView::on_current(optional<int> current) {
  update_focus(current);
  if(m_current_entry && m_current_entry->m_index != current) {
    m_current_entry->m_item.set_current(false);
    m_current_entry = nullptr;
  }
  if(focusPolicy() != Qt::NoFocus &&
      find_focus_state(*this) != FocusObserver::State::NONE) {
    setFocus();
  }
  if(current) {
    m_current_entry = m_items[*current].get();
    m_current_entry->m_item.set_current(true);
  }
}

void ListView::on_selection(const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const ListModel<int>::AddOperation& operation) {
      auto& selection = m_selection_controller.get_selection();
      m_items[selection->get(operation.m_index)]->m_item.set_selected(true);
    },
    [&] (const ListModel<int>::PreRemoveOperation& operation) {
      auto& selection = m_selection_controller.get_selection();
      auto index = selection->get(operation.m_index);
      if(index < static_cast<int>(m_items.size())) {
        m_items[index]->m_item.set_selected(false);
      }
    },
    [&] (const ListModel<int>::UpdateOperation& operation) {
      if(operation.get_previous() < static_cast<int>(m_items.size())) {
        m_items[operation.get_previous()]->m_item.set_selected(false);
      }
      m_items[operation.get_value()]->m_item.set_selected(true);
    });
}

void ListView::on_item_submitted(ItemEntry& item) {
  if(m_current_controller.get_current()->get() != item.m_index) {
    m_current_controller.get_current()->set(item.m_index);
  }
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
          if(m_overflow != overflow) {
            m_overflow = overflow;
            if(isVisible()) {
              initialize_visible_region();
            }
            *has_update = true;
          }
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
