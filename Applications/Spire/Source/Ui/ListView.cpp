#include "Spire/Ui/ListView.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"

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
    std::function<ListItem* (
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
      m_current_index(-1),
      m_column_or_row_index(0),
      m_is_setting_item_focus(false) {
  set_style(*this, DEFAULT_STYLE());
  connect_style_signal(*this, [=] { update_layout(); });
  m_items.resize(m_list_model->get_size());
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    auto value = m_list_model->get<QString>(i);
    auto list_item = m_factory(m_list_model, i);
    m_items[i] = {list_item, value, connect_item_current(list_item, value),
      connect_item_submit(list_item, value)};
  }
  m_list_model_connection = m_list_model->connect_operation_signal(
    [=] (const ListModel::Operation& operation) {
      visit(operation,
        [&] (const ListModel::RemoveOperation& operation) {
          on_delete_item(operation.m_index);
        });
  });
  connect(&m_query_timer, &QTimer::timeout, this, [=] { m_query.clear(); });
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
}

connection ListView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void ListView::keyPressEvent(QKeyEvent* event) {
  if(m_selection_mode == SelectionMode::NONE || m_list_model->get_size() == 0) {
    QWidget::keyPressEvent(event);
    return;
  }
  switch(event->key()) {
  case Qt::Key_Home:
  case Qt::Key_PageUp:
    update_current_item(0, true);
    break;
  case Qt::Key_End:
  case Qt::Key_PageDown:
    update_current_item(m_list_model->get_size() - 1, true);
    break;
  case Qt::Key_Down:
    if(m_direction == Qt::Horizontal && m_overflow == Overflow::WRAP) {
      auto row_height =
        layout()->itemAt(m_column_or_row_index)->geometry().height();
      if(m_tracking_position.y() + row_height <
          layout()->itemAt(layout()->count() - 2)->geometry().bottom()) {
        m_tracking_position.setY(m_tracking_position.y() + row_height +
          layout()->itemAt(m_column_or_row_index + 1)->geometry().height());
        cross_move(true);
        break;
      }
    }
    update_current_item(move_next(), true);
    break;
  case Qt::Key_Up:
    if(m_direction == Qt::Horizontal && m_overflow == Overflow::WRAP) {
      if(m_tracking_position.y() != rect().y()) {
        m_tracking_position.setY(m_tracking_position.y() -
          layout()->itemAt(m_column_or_row_index - 2)->geometry().height() -
          layout()->itemAt(m_column_or_row_index - 1)->geometry().height());
        cross_move(false);
        break;
      }
    }
    update_current_item(move_previous(), true);
    break;
  case Qt::Key_Left:
    if(m_direction == Qt::Vertical && m_overflow == Overflow::WRAP) {
      if(m_tracking_position.x() != rect().x()) {
        m_tracking_position.setX(m_tracking_position.x() -
          layout()->itemAt(m_column_or_row_index - 2)->geometry().width() -
          layout()->itemAt(m_column_or_row_index - 1)->geometry().width());
        cross_move(false);
        break;
      }
    }
    update_current_item(move_previous(), true);
    break;
  case Qt::Key_Right:
    if(m_direction == Qt::Vertical && m_overflow == Overflow::WRAP) {
      auto column_width =
        layout()->itemAt(m_column_or_row_index)->geometry().width();
      if(m_tracking_position.x() + column_width <
          layout()->itemAt(layout()->count() - 2)->geometry().right()) {
        m_tracking_position.setX(m_tracking_position.x() + column_width +
          layout()->itemAt(m_column_or_row_index + 1)->geometry().width());
        cross_move(true);
        break;
      }
    }
    update_current_item(move_next(), true);
    break;
  default:
    auto key = event->text();
    if(!key.isEmpty() && key[0].isLetterOrNumber()) {
      m_query += key.toUpper();
      m_query_timer.start(QUERY_TIMEOUT_MS);
      query();
    }
    break;
  }
  QWidget::keyPressEvent(event);
}

void ListView::resizeEvent(QResizeEvent* event) {
  update_layout();
}

scoped_connection ListView::connect_item_current(ListItem* item,
    const QString& value) {
  return item->connect_current_signal([=] {
    if(m_selection_mode == SelectionMode::NONE) {
      return;
    }
    if(!m_is_setting_item_focus) {
      m_current_index = get_index_by_value(value);
      update_tracking_position();
    }
    m_is_setting_item_focus = false;
    if(m_current_index == -1) {
      m_current_model->set_current(boost::none);
    } else {
      m_current_model->set_current(value);
    }
  });
}

scoped_connection ListView::connect_item_submit(ListItem* item,
    const QString& value) {
  return item->connect_submit_signal([=] {
    m_current_index = get_index_by_value(value);
    update_tracking_position();
    m_submit_signal(value);
  });
}

int ListView::get_index_by_value(const QString& value) {
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(m_list_model->get<QString>(i) == value) {
      return i;
    }
  }
  return -1;
}

void ListView::cross_move(bool is_next) {
  m_column_or_row_index = [=] {
    if(is_next) {
      return m_column_or_row_index + 2;
    } else {
      return m_column_or_row_index - 2;
    }
  }();
  auto target = layout()->itemAt(m_column_or_row_index)->layout();
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
        if(std::abs(v0 - v1) < min_value) {
          index = i;
        } else if(i == 0) {
          index = i;
        } else {
          index = i - 2;
        }
        break;
      } else {
        min_value = std::abs(v0 - v2);
      }
    }
  }
  auto item = target->itemAt(index)->widget();
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    if(item == m_items[i].m_component) {
      update_current_item(i, false);
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

void ListView::on_delete_item(int index) {
  delete m_items[index].m_component;
  m_items.erase(std::next(m_items.begin(), index));
  if(m_current_index >= m_list_model->get_size()) {
    m_current_index = m_list_model->get_size() - 1;
  }
  update_current_item(m_current_index, true);
}

void ListView::update_column_row_index() {
  m_column_or_row_index = -1;
  if(auto list_view_layout = layout()) {
    for(auto i = 0; i < list_view_layout->count(); i += 2) {
      if(auto child_layout = list_view_layout->itemAt(i)->layout()) {
        if(child_layout->indexOf(m_items[m_current_index].m_component) >= 0) {
          m_column_or_row_index = i;
          break;
        }
      }
    }
  }
}

void ListView::update_current_item(int index, bool is_update_x_y) {
  if(m_selection_mode != SelectionMode::NONE && index != -1) {
    m_is_setting_item_focus = true;
    m_current_index = index;
    m_items[index].m_component->setFocus();
    if(is_update_x_y) {
      update_tracking_position();
    }
  }
}

void ListView::update_layout() {
  delete layout();
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
      return widget->height();
    } else {
      return widget->width();
    }
  };
  auto get_dimension = [=] {
    if(m_direction == Qt::Vertical) {
      return height();
    } else {
      return width();
    }
  };
  auto layout = [=] () -> QBoxLayout* {
    if(m_direction == Qt::Vertical) {
      return new QHBoxLayout(this);
    } else {
      return new QVBoxLayout(this);
    }
  }();
  layout->setSpacing(0);
  layout->setContentsMargins({});
  if(m_overflow == Overflow::NONE) {
    auto child_layout = get_child_layout();
    child_layout->addWidget(m_items[0].m_component);
    for(auto i = 1; i < m_list_model->get_size(); ++i) {
      child_layout->addSpacing(gap);
      child_layout->addWidget(m_items[i].m_component);
    }
    layout->addLayout(child_layout);
  } else {
    auto child_layout = get_child_layout();
    auto first_item = m_items[0].m_component;
    child_layout->addWidget(first_item);
    auto child_dimension = get_item_dimension(first_item);
    for(auto i = 1; i < m_list_model->get_size(); ++i) {
      auto item = m_items[i].m_component;
      child_dimension += get_item_dimension(item) + gap;
      if(child_dimension <= get_dimension()) {
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
  adjustSize();
  update_tracking_position();
}

void ListView::update_tracking_position() {
  if(m_current_index == -1) {
    return;
  }
  m_tracking_position = m_items[m_current_index].m_component->pos();
  update_column_row_index();
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
    if(auto value = m_list_model->get<QString>(index).toUpper();
        value.startsWith(query)) {
      update_current_item(index, true);
      break;
    }
    index = (index + 1) % item_count;
    ++count;
  }
}
