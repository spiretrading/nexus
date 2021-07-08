#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Box.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  const auto DEFAULT_GAP = 0;
  const auto DEFAULT_OVERFLOW_GAP = DEFAULT_GAP;
  const auto QUERY_TIMEOUT_MS = 500;

  auto DEFAULT_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(ListItemGap(scale_height(DEFAULT_GAP))).
      set(ListOverflowGap(scale_width(DEFAULT_OVERFLOW_GAP)));
    return style;
  }
}

ListView::ListView(std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<ArrayListModel> list_model,
    std::function<QWidget* (
      std::shared_ptr<ArrayListModel>, int index)> factory,
    QWidget* parent)
    : QWidget(parent),
      m_current_model(std::move(current_model)),
      m_list_model(std::move(list_model)),
      m_factory(std::move(factory)),
      m_direction(Qt::Vertical),
      m_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_selection_mode(SelectionMode::SINGLE),
      m_is_selection_follows_focus(true),
      m_current_index(-1),
      m_column_or_row_index(-1),
      m_is_setting_item_focus(false) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_body = new QWidget(this);
  auto box = new Box(m_body);
  layout->addWidget(box);
  set_style(*this, DEFAULT_STYLE());
  connect_style_signal(*this, [=] { update_layout(); });
  m_items.resize(m_list_model->get_size());
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto value = m_list_model->get<QString>(i);
    auto list_item = new ListItem(m_factory(m_list_model, i), this);
    m_items[i] = {list_item, connect_item_current(list_item, value),
      connect_item_submit(list_item, value)};
  }
  m_list_model_connection = m_list_model->connect_operation_signal(
    [=] (const ListModel::Operation& operation) {
      visit(operation,
        [&] (const ListModel::AddOperation& operation) {
          on_add_item(operation.m_index);
        },
        [&] (const ListModel::RemoveOperation& operation) {
          on_delete_item(operation.m_index);
        });
  });
  m_current_connection = m_current_model->connect_current_signal(
    [=] (const auto& current) {
      on_current(current);
    });
  connect(&m_query_timer, &QTimer::timeout, this, [=] { m_query.clear(); });
  update_layout();
}

const std::shared_ptr<ListView::CurrentModel>&
    ListView::get_current_model() const {
  return m_current_model;
}

const std::shared_ptr<ArrayListModel>& ListView::get_list_model() const {
  return m_list_model;
}

Qt::Orientation ListView::get_direction() const {
  return m_direction;
}

void ListView::set_direction(Qt::Orientation direction) {
  m_direction = direction;
  update_layout();
}

ListView::EdgeNavigation ListView::get_edge_navigation() const {
  return m_navigation;
}

void ListView::set_edge_navigation(EdgeNavigation navigation) {
  m_navigation = navigation;
}

ListView::Overflow ListView::get_overflow() const {
  return m_overflow;
}

void ListView::set_overflow(Overflow overflow) {
  m_overflow = overflow;
  update_layout();
}

ListView::SelectionMode ListView::get_selection_mode() const {
  return m_selection_mode;
}

void ListView::set_selection_mode(SelectionMode selection_mode) {
  m_selection_mode = selection_mode;
  if(m_selection_mode == SelectionMode::NONE && !m_selected.isEmpty()) {
    deselect_item();
  }
}

bool ListView::get_selection_follows_focus() const {
  return m_is_selection_follows_focus;
}

void ListView::set_selection_follows_focus(bool is_selection_follows_focus) {
  m_is_selection_follows_focus = is_selection_follows_focus;
}

const QString& ListView::get_selected() const {
  return m_selected;
}

const ListItem* ListView::get_item(const QString& value) const {
  auto index = get_index_by_value(value);
  if(index < 0 || index >= m_list_model->get_size()) {
    return nullptr;
  }
  return m_items[index].m_item;
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool ListView::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    adjustSize();
  }
  return QWidget::event(event);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  if(m_list_model->get_size() == 0) {
    QWidget::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
  case Qt::Key_Home:
  case Qt::Key_PageUp:
    update_current(0);
    break;
  case Qt::Key_End:
  case Qt::Key_PageDown:
    update_current(m_list_model->get_size() - 1);
    break;
  case Qt::Key_Down:
    if(m_direction == Qt::Horizontal && m_overflow == Overflow::WRAP) {
      auto row_height =
        get_column_or_row(m_column_or_row_index)->geometry().height();
      if(m_tracking_position.y() + row_height <
          get_column_or_row(get_layout()->count() - 2)->geometry().bottom()) {
        m_tracking_position.setY(m_tracking_position.y() + row_height +
          get_column_or_row(m_column_or_row_index + 1)->geometry().height());
        cross_move(true);
        break;
      }
    }
    update_current(move_next());
    break;
  case Qt::Key_Up:
    if(m_direction == Qt::Horizontal && m_overflow == Overflow::WRAP) {
      if(m_tracking_position.y() != rect().y()) {
        m_tracking_position.setY(m_tracking_position.y() -
          get_column_or_row(m_column_or_row_index - 2)->geometry().height() -
          get_column_or_row(m_column_or_row_index - 1)->geometry().height());
        cross_move(false);
        break;
      }
    }
    update_current(move_previous());
    break;
  case Qt::Key_Left:
    if(m_direction == Qt::Vertical && m_overflow == Overflow::WRAP) {
      if(m_tracking_position.x() != rect().x()) {
        m_tracking_position.setX(m_tracking_position.x() -
          get_column_or_row(m_column_or_row_index - 2)->geometry().width() -
          get_column_or_row(m_column_or_row_index - 1)->geometry().width());
        cross_move(false);
        break;
      }
    }
    update_current(move_previous());
    break;
  case Qt::Key_Right:
    if(m_direction == Qt::Vertical && m_overflow == Overflow::WRAP) {
      auto column_width =
        get_column_or_row(m_column_or_row_index)->geometry().width();
      if(m_tracking_position.x() + column_width <
          get_column_or_row(get_layout()->count() - 2)->geometry().right()) {
        m_tracking_position.setX(m_tracking_position.x() + column_width +
          get_column_or_row(m_column_or_row_index + 1)->geometry().width());
        cross_move(true);
        break;
      }
    }
    update_current(move_next());
    break;
  default:
    auto key = event->text();
    if(!key.isEmpty() && key[0].isLetterOrNumber()) {
      m_query += key.toLower();
      m_query_timer.start(QUERY_TIMEOUT_MS);
      query();
    }
    break;
  }
  QWidget::keyPressEvent(event);
}

void ListView::resizeEvent(QResizeEvent* event) {
  if(event->size() != event->oldSize()) {
    update_layout();
  }
}

scoped_connection ListView::connect_item_current(ListItem* item,
    const QString& value) {
  return item->connect_current_signal([=] {
    if(!m_is_setting_item_focus) {
      m_current_index = get_index_by_value(value);
      if(m_current_index == -1) {
        m_current_model->set_current(boost::none);
      } else {
        m_current_model->set_current(value);
      }
      update_tracking_position();
    }
    m_is_setting_item_focus = false;
  });
}

scoped_connection ListView::connect_item_submit(ListItem* item,
    const QString& value) {
  return item->connect_submit_signal([=] {
    m_current_index = get_index_by_value(value);
    update_tracking_position();
    if(!m_is_selection_follows_focus) {
      update_selection(value, m_current_index);
    }
    m_submit_signal(value);
  });
}

int ListView::get_index_by_value(const QString& value) const {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(m_list_model->get<QString>(i) == value) {
      return i;
    }
  }
  return -1;
}

QLayout* Spire::ListView::get_layout() const {
  return m_body->layout();
}

QLayoutItem* ListView::get_column_or_row(int index) const {
  return get_layout()->itemAt(index);
}

void ListView::deselect_item() {
  if(auto index = get_index_by_value(m_selected); index != -1) {
    m_items[index].m_item->set_selected(false);
  }
}

void ListView::cross_move(bool is_next) {
  m_column_or_row_index = [=] {
    if(is_next) {
      return m_column_or_row_index + 2;
    } else {
      return m_column_or_row_index - 2;
    }
  }();
  auto target = get_column_or_row(m_column_or_row_index)->layout();
  auto [v0, min_value] = [=] () -> std::tuple<int, int> {
    if(m_direction == Qt::Vertical) {
      return {m_tracking_position.y(),
        std::abs(target->itemAt(0)->geometry().bottom() -
          m_tracking_position.y())};
    } else {
      return {m_tracking_position.x(),
        std::abs(target->itemAt(0)->geometry().right() -
          m_tracking_position.x())};
    }
  }();
  auto index = 0;
  for(auto i = 0; i < target->count(); i += 2) {
    auto [v1, v2] = [=] () -> std::tuple<int, int> {
      auto item_geometry = target->itemAt(i)->geometry();
      if(m_direction == Qt::Vertical) {
        return {item_geometry.top(), item_geometry.bottom()};
      } else {
        return {item_geometry.left(), item_geometry.right()};
      }
    }();
    if(v0 >= v1 && v0 <= v2) {
      index = i;
      break;
    } else {
      if(v1 > v0) {
        if(std::abs(v0 - v1) < min_value || i == 0) {
          index = i;
        } else {
          index = i - 2;
        }
        break;
      } else {
        min_value = std::abs(v0 - v2);
        index = i;
      }
    }
  }
  auto item = target->itemAt(index)->widget();
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(item == m_items[i].m_item) {
      update_current(i, false, false);
      break;
    }
  }
}

int ListView::move_next() {
  if(m_navigation == EdgeNavigation::CONTAIN) {
    return std::min(m_current_index + 1, m_list_model->get_size() - 1);
  } else {
    return (m_current_index + 1) % m_list_model->get_size();
  }
}

int ListView::move_previous() {
  if(m_navigation == EdgeNavigation::CONTAIN) {
    return std::max(m_current_index - 1, 0);
  } else {
    auto index = (m_current_index - 1) % m_list_model->get_size();
    if(index < 0) {
      index = m_list_model->get_size() - 1;
    }
    return index;
  }
}

void ListView::on_current(const optional<QString>& current) {
  if(m_current_index != -1 && !m_items[m_current_index].m_item->hasFocus()) {
    m_is_setting_item_focus = true;
    m_items[m_current_index].m_item->setFocus();
  }
  if(m_is_selection_follows_focus) {
    if(current) {
      update_selection(*current, m_current_index);
    } else {
      update_selection("", -1);
    }
  }
}

void ListView::on_add_item(int index) {
  auto value = m_list_model->get<QString>(index);
  auto list_item = new ListItem(m_factory(m_list_model, index), this);
  m_items.insert(m_items.begin() + index, {list_item,
    connect_item_current(list_item, value),
    connect_item_submit(list_item, value)});
  update_after_items_changed();
}

void ListView::on_delete_item(int index) {
  delete m_items[index].m_item;
  m_items.erase(std::next(m_items.begin(), index));
  update_after_items_changed();
}

void ListView::update_column_row_index() {
  m_column_or_row_index = -1;
  if(auto layout = get_layout()) {
    for(auto i = 0; i < layout->count(); i += 2) {
      if(auto child_layout = layout->itemAt(i)->layout();
          child_layout->indexOf(m_items[m_current_index].m_item) >= 0) {
        m_column_or_row_index = i;
        break;
      }
    }
  }
}

void ListView::update_layout() {
  if(m_body->layout()) {
    delete m_body->layout();
  }
  if(m_items.empty()) {
    return;
  }
  auto gap = DEFAULT_GAP;
  auto overflow_gap = DEFAULT_OVERFLOW_GAP;
  auto& stylist = find_stylist(*this);
  auto block = stylist.get_computed_block();
  if(auto gap_property = Styles::find<ListItemGap>(block)) {
    stylist.evaluate(*gap_property, [&] (auto value) {
      gap = value;
    });
  }
  if(auto overflow_gap_property = Styles::find<ListOverflowGap>(block)) {
    stylist.evaluate(*overflow_gap_property, [&] (auto value) {
      overflow_gap = value;
    });
  }
  auto get_child_layout = [=] () -> QBoxLayout* {
    auto layout = [=] () -> QBoxLayout* {
      if(m_direction == Qt::Vertical) {
        return new QVBoxLayout();
      } else {
        return new QHBoxLayout();
      }
    }();
    layout->setSpacing(0);
    layout->setContentsMargins({});
    return layout;
  };
  auto get_item_dimension = [=] (QWidget* widget) {
    if(m_direction == Qt::Vertical) {
      return widget->sizeHint().height();
    } else {
      return widget->sizeHint().width();
    }
  };
  auto dimension = [=] {
    if(m_direction == Qt::Vertical) {
      return height();
    } else {
      return width();
    }
  }();
  auto layout = [=] () -> QBoxLayout* {
    if(m_direction == Qt::Vertical) {
      return new QHBoxLayout(m_body);
    } else {
      return new QVBoxLayout(m_body);
    }
  }();
  layout->setSpacing(0);
  layout->setContentsMargins({});
  if(m_overflow == Overflow::NONE) {
    auto child_layout = get_child_layout();
    child_layout->addWidget(m_items[0].m_item);
    for(auto i = 1; i < m_list_model->get_size(); ++i) {
      child_layout->addSpacing(gap);
      child_layout->addWidget(m_items[i].m_item);
    }
    layout->addLayout(child_layout);
  } else {
    auto child_layout = get_child_layout();
    auto first_item = m_items[0].m_item;
    child_layout->addWidget(first_item);
    auto child_dimension = get_item_dimension(first_item);
    for(auto i = 1; i < m_list_model->get_size(); ++i) {
      auto item = m_items[i].m_item;
      child_dimension += get_item_dimension(item) + gap;
      if(child_dimension <= dimension) {
        child_layout->addSpacing(gap);
        child_layout->addWidget(item);
      } else {
        child_layout->addStretch();
        layout->addLayout(child_layout);
        layout->addSpacing(overflow_gap);
        child_layout = get_child_layout();
        child_layout->addWidget(item);
        child_dimension = get_item_dimension(item);
      }
    }
    child_layout->addStretch();
    layout->addLayout(child_layout);
    layout->addStretch();
  }
  m_body->adjustSize();
  update_tracking_position();
}

void ListView::update_tracking_position() {
  if(m_current_index == -1) {
    return;
  }
  m_tracking_position = m_items[m_current_index].m_item->pos();
  update_column_row_index();
}

void ListView::update_current(int index, bool is_forced_update,
    bool is_update_x_y) {
  if(!is_forced_update && m_current_index == index) {
    return;
  }
  m_current_index = index;
  if(m_current_index != -1) {
    m_current_model->set_current(m_list_model->get<QString>(m_current_index));
  }
  if(is_update_x_y) {
    update_tracking_position();
  }
}

void ListView::update_current(int index) {
  update_current(index, false, true);
}

void ListView::update_after_items_changed() {
  if(m_current_index >= m_list_model->get_size()) {
    m_current_index = m_list_model->get_size() - 1;
  }
  update_layout();
  update_current(m_current_index, true, true);
}

void ListView::update_selection(const QString& selected, int selected_index) {
  if(m_selection_mode == SelectionMode::NONE) {
    return;
  }
  deselect_item();
  m_selected = selected;
  if(selected_index != -1) {
    m_items[selected_index].m_item->set_selected(true);
  }
}

void ListView::query() {
  if(m_query.isEmpty()) {
    return;
  }
  auto item_count = m_list_model->get_size();
  auto [query, index] = [=] () -> std::tuple<QString, int> {
    if(m_query.count(m_query.front()) == m_query.size()) {
      return {static_cast<QString>(m_query[0]),
        (m_current_index + 1) % item_count};
    }
    return {m_query, m_current_index};
  }();
  auto count = 0;
  while(count < item_count) {
    if(auto value = m_list_model->get<QString>(index).toLower();
        value.startsWith(query)) {
      update_current(index);
      break;
    }
    index = (index + 1) % item_count;
    ++count;
  }
}
