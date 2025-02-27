#include "Spire/Ui/ListCurrentController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

ListCurrentController::ListCurrentController(
  std::shared_ptr<CurrentModel> current, int size)
  : m_current(std::move(current)),
    m_size(size),
    m_edge_navigation(EdgeNavigation::WRAP),
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
  auto update_current = is_initialized();
  m_views.insert(std::next(m_views.begin(), index), std::move(view));
  m_size = std::max<int>(m_size, std::ssize(m_views));
  if(!update_current) {
    return;
  }
  if(m_current->get() && *m_current->get() == m_size - 1 ||
      index <= m_current->get().value_or(-1)) {
    on_current(m_current->get());
  }
}

void ListCurrentController::remove(int index) {
  m_views.erase(m_views.begin() + index);
  --m_size;
  if(index <= m_current->get().value_or(-1)) {
    on_current(m_current->get());
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
    std::rotate(std::next(m_views.rbegin(), m_views.size() - source - 1),
      std::next(m_views.rbegin(), m_views.size() - source),
      std::next(m_views.rbegin(), m_views.size() - destination));
  } else {
    std::rotate(std::next(m_views.begin(), source),
      std::next(m_views.begin(), source + 1),
      std::next(m_views.begin(), destination + 1));
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
  auto current_navigation_box = [&] {
    if(m_current->get()) {
      return m_views[*m_current->get()]->get_geometry();
    }
    return m_views.front()->get_geometry();
  }();
  auto i = *m_current->get() + direction;
  auto navigation_box = current_navigation_box;
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
          direction * current_navigation_box.y() <
            direction * navigation_box.y() ||
          orientation == Qt::Orientation::Vertical &&
          direction * current_navigation_box.x() <
            direction * navigation_box.x()) &&
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
  m_current->set(candidate);
}

connection ListCurrentController::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

bool ListCurrentController::is_initialized() const {
  return m_size == m_views.size();
}

void ListCurrentController::on_current(optional<int> current) {
  if(current && *current < m_size) {
    m_update_signal(current);
  }
}
