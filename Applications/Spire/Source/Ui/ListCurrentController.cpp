#include "Spire/Ui/ListCurrentController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListCurrentController::ListCurrentController(
  std::shared_ptr<CurrentModel> current)
  : m_current(std::move(current)),
    m_edge_navigation(EdgeNavigation::WRAP),
    m_last_current(m_current->get()),
    m_connection(m_current->connect_update_signal(
      std::bind_front(&ListCurrentController::on_current, this))) {}

const std::shared_ptr<ListCurrentController::CurrentModel>&
    ListCurrentController::get_current() const {
  return m_current;
}

ListCurrentController::EdgeNavigation
    ListCurrentController::get_edge_navigation() const {
  return m_edge_navigation;
}

void ListCurrentController::set_edge_navigation(EdgeNavigation navigation) {
  m_edge_navigation = navigation;
}

void ListCurrentController::add(std::unique_ptr<ItemView> view, int index) {
  m_views.insert(std::next(m_views.begin(), index), std::move(view));
  if(m_current->get() && *m_current->get() >= index) {
    auto current = *m_current->get() + 1;
    update(current);
    auto blocker = shared_connection_block(m_connection);
    m_current->set(current);
  }
}

void ListCurrentController::remove(int index) {
  m_views.erase(m_views.begin() + index);
  if(m_current->get()) {
    if(m_current->get() == index) {
      auto size = static_cast<int>(m_views.size());
      if(size == 0) {
        m_current->set(none);
      } else if(index >= size) {
        m_current->set(size - 1);
      } else {
        m_current->set(index);
      }
    } else if(m_current->get() > index) {
      auto current = *m_current->get() - 1;
      update(current);
      auto blocker = shared_connection_block(m_connection);
      m_current->set(current);
    }
  }
}

void ListCurrentController::move(int source, int destination) {
  if(source == destination) {
    return;
  }
  auto direction = [&] {
    if(source < destination) {
      return -1;
    }
    return 1;
  }();
  if(direction == 1) {
    std::rotate(m_views.begin() + destination, m_views.begin() + source,
      m_views.begin() + source + 1);
  } else {
    auto source_index = m_views.size() - 1 - source;
    auto destination_index = m_views.size() - 1 - destination;
    std::rotate(m_views.rbegin() + destination_index,
      m_views.rbegin() + source_index,
      m_views.rbegin() + source_index + 1);
  }
  auto adjust = [&] (auto& value) {
    if(value && (*value >= source || *value <= destination)) {
      *value += direction;
      return true;
    }
    return false;
  };
  adjust(m_last_current);
  auto current = m_current->get();
  if(adjust(current)) {
    update(current);
    auto blocker = shared_connection_block(m_connection);
    m_current->set(current);
  }
}

void ListCurrentController::navigate_home() {
  navigate(1, -1, EdgeNavigation::CONTAIN);
}

void ListCurrentController::navigate_end() {
  navigate(-1, static_cast<int>(m_views.size()), EdgeNavigation::CONTAIN);
}

void ListCurrentController::navigate_next() {
  navigate(1, m_current->get().value_or(-1), m_edge_navigation);
}

void ListCurrentController::navigate_previous() {
  if(m_current->get()) {
    navigate(-1, *m_current->get(), m_edge_navigation);
  } else {
    navigate_next();
  }
}

void ListCurrentController::navigate(int direction, int start) {
  navigate(direction, start, m_edge_navigation);
}

void ListCurrentController::navigate(
    int direction, int start, EdgeNavigation edge_navigation) {
  if(m_views.empty()) {
    return;
  }
  auto i = start;
  do {
    i += direction;
    if(i < 0 || i >= static_cast<int>(m_views.size())) {
      if(edge_navigation == EdgeNavigation::CONTAIN) {
        return;
      } else if(direction == -1) {
        i = static_cast<int>(m_views.size()) - 1;
      } else {
        if(start == -1) {
          return;
        }
        i = 0;
      }
    }
  } while(i != start && !m_views[i]->is_selectable());
  m_current->set(i);
}

void ListCurrentController::cross_next(Qt::Orientation orientation) {
  cross(1, orientation);
}

void ListCurrentController::cross_previous(Qt::Orientation orientation) {
  cross(-1, orientation);
}

void ListCurrentController::cross(int direction, Qt::Orientation orientation) {
  if(m_views.empty()) {
    return;
  }
  if(!m_current->get()) {
    navigate_next();
    return;
  }
  if(m_navigation_box.isNull()) {
    if(m_current->get()) {
      m_navigation_box = m_views[*m_current->get()]->get_geometry();
    } else {
      m_navigation_box = m_views.front()->get_geometry();
    }
  }
  auto i = *m_current->get() + direction;
  auto navigation_box = m_navigation_box;
  auto candidate = -1;
  while(i >= 0 && i != static_cast<int>(m_views.size())) {
    if(m_views[i]->is_selectable()) {
      if(candidate == -1) {
        if(orientation == Qt::Orientation::Horizontal) {
          navigation_box.moveTop(m_views[i]->get_geometry().y());
        } else {
          navigation_box.moveLeft(m_views[i]->get_geometry().x());
        }
      }
      if((orientation == Qt::Orientation::Horizontal &&
          direction * m_navigation_box.y() < direction * navigation_box.y() ||
          orientation == Qt::Orientation::Vertical &&
          direction * m_navigation_box.x() < direction * navigation_box.x()) &&
          navigation_box.intersects(m_views[i]->get_geometry())) {
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
  m_current->set(candidate);
  if(candidate == m_current->get()) {
    m_navigation_box = navigation_box;
  }
}

connection ListCurrentController::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void ListCurrentController::update(optional<int> current) {
  m_last_current = current;
  if(current) {
    m_navigation_box = m_views[*current]->get_geometry();
  } else {
    m_navigation_box = QRect();
  }
}

void ListCurrentController::on_current(optional<int> current) {
  auto previous = m_last_current;
  update(current);
  m_update_signal(previous, current);
}
