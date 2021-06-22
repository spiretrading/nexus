#include "Spire/Ui/ListView.hpp"
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
}

ListView::ListView(std::shared_ptr<CurrentModel> current_model,
    std::shared_ptr<ListModel> list_model,
    std::function<ListItem* (std::shared_ptr<ListModel>, int index)> factory,
    QWidget* parent)
    : QWidget(parent),
      m_current_model(std::move(current_model)),
      m_list_model(std::move(list_model)),
      m_factory(std::move(factory)),
      m_direction(Qt::Vertical),
      m_navigation(EdgeNavigation::WRAP),
      m_overflow(Overflow::NONE),
      m_gap(0),
      m_overflow_gap(m_gap),
      m_current_index(-1) {
  m_items.resize(m_list_model->get_size());
  for(auto i = 0; i < m_list_model->get_size(); ++i) {
    m_items[i] = m_factory(m_list_model, i);
    m_items[i]->connect_current_signal([=] {
      m_current_index = static_cast<int>(i);
      m_current_model->set_current(m_list_model->get<QString>(m_current_index));
    });
  }
  m_current_model->connect_current_signal([=] (const auto& value) {
    if(!value) {
      return;
    }
    if(m_overflow == Overflow::WRAP) {
      auto list_view_layout = layout();
      for(auto i = 0; i < list_view_layout->count(); ++i) {
        if(auto child_layout = list_view_layout->itemAt(i)->layout()) {
          if(child_layout->indexOf(m_items[m_current_index]) >= 0) {
            m_column_or_row_index = i;
            return;
          }
        }
      }
      m_column_or_row_index = -1;
    }
  });
  connect(&m_query_timer, &QTimer::timeout, this, [=] { m_query.clear(); });
  update_layout();
}

const std::shared_ptr<ListView::CurrentModel>&
    ListView::get_current_model() const {
  return m_current_model;
}

const std::shared_ptr<ListModel>& ListView::get_list_model() const {
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

connection ListView::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

connection ListView::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

void ListView::keyPressEvent(QKeyEvent* event) {
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
    if(m_direction == Qt::Vertical) {
      update_current(move_next());
    } else if(m_overflow == Overflow::WRAP) {
      if(m_current_index < 0) {
        return;
      }
      if(m_items[m_current_index]->y() +
          layout()->itemAt(m_column_or_row_index)->geometry().height() <
          rect().bottom()) {
        select_nearest_item(true);
      } else {
        update_current(move_next());
      }
    }
    break;
  case Qt::Key_Up:
    if(m_direction == Qt::Vertical) {
      update_current(move_previous());
    } else if(m_overflow == Overflow::WRAP) {
      if(m_current_index < 0) {
        return;
      }
      if(m_items[m_current_index]->y() != rect().y()) {
        select_nearest_item(false);
      } else {
        update_current(move_previous());
      }
    }
    break;
  case Qt::Key_Left:
    if(m_direction == Qt::Horizontal) {
      update_current(move_previous());
    } else if(m_overflow == Overflow::WRAP) {
      if(m_current_index < 0) {
        return;
      }
      if(m_items[m_current_index]->x() != rect().x()) {
        select_nearest_item(false);
      } else {
        update_current(move_previous());
      }
    }
    break;
  case Qt::Key_Right:
    if(m_direction == Qt::Horizontal) {
      update_current(move_next());
    } else if(m_overflow == Overflow::WRAP) {
      if(m_current_index < 0) {
        return;
      }
      if(m_items[m_current_index]->x() +
          layout()->itemAt(m_column_or_row_index)->geometry().width() <
          rect().right()) {
        select_nearest_item(true);
      } else {
        update_current(move_next());
      }
    }
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
}

void ListView::resizeEvent(QResizeEvent* event) {
  update_layout();
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

void ListView::update_current(int index) {
  m_items[index]->setFocus();
}

void ListView::update_layout() {
  delete layout();
  if(m_items.empty()) {
    return;
  }
  if(m_direction == Qt::Vertical) {
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      column_layout->addWidget(m_items.front());
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        column_layout->addSpacing(m_gap);
        column_layout->addWidget(*i);
      }
      layout->addLayout(column_layout);
    } else {
      auto column_height = 0;
      auto column_layout = new QVBoxLayout();
      column_layout->setSpacing(0);
      column_layout->setContentsMargins({});
      column_layout->addWidget(m_items.front(), 0, Qt::AlignTop);
      column_height += m_items.front()->height();
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        column_height += (*i)->height() + m_gap;
        if(column_height <= height()) {
          column_layout->addSpacing(m_gap);
          column_layout->addWidget(*i, 0, Qt::AlignTop);
        } else {
          column_layout->addStretch();
          layout->addLayout(column_layout);
          layout->addSpacing(m_overflow_gap);
          column_layout = new QVBoxLayout();
          column_layout->setSpacing(0);
          column_layout->setContentsMargins({});
          column_layout->addWidget(*i, 0, Qt::AlignTop);
          column_height = (*i)->height();
        }
      }
      column_layout->addStretch();
      layout->addLayout(column_layout);
    }
  } else {
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins({});
    if(m_overflow == Overflow::NONE) {
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      row_layout->addWidget(m_items.front(), 0, Qt::AlignTop);
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        row_layout->addSpacing(m_gap);
        row_layout->addWidget(*i, 0, Qt::AlignTop);
      }
      layout->addLayout(row_layout);
    } else {
      auto row_width = 0;
      auto row_layout = new QHBoxLayout();
      row_layout->setSpacing(0);
      row_layout->setContentsMargins({});
      row_layout->addWidget(m_items.front(), 0, Qt::AlignLeft | Qt::AlignTop);
      row_width += m_items.front()->width();
      for(auto i = m_items.begin() + 1; i != m_items.end(); ++i) {
        row_width += (*i)->width() + m_gap;
        if(row_width <= width()) {
          row_layout->addSpacing(m_gap);
          row_layout->addWidget(*i, 0, Qt::AlignLeft | Qt::AlignTop);
        } else {
          row_layout->addStretch();
          layout->addLayout(row_layout);
          layout->addSpacing(m_overflow_gap);
          row_layout = new QHBoxLayout();
          row_layout->setSpacing(0);
          row_layout->setContentsMargins({});
          row_layout->addWidget(*i, 0, Qt::AlignLeft | Qt::AlignTop);
          row_width = (*i)->width();
        }
      }
      row_layout->addStretch();
      layout->addLayout(row_layout);
    }
  }
  adjustSize();
}

void ListView::select_nearest_item(bool is_next) {
  auto column_layout = [=] {
    if(is_next) {
      return layout()->itemAt(m_column_or_row_index + 2)->layout();
    } else {
      return layout()->itemAt(m_column_or_row_index - 2)->layout();
    }
  }();
  auto item_pos = [=] {
    if(m_direction == Qt::Vertical) {
      return m_items[m_current_index]->y();
    } else {
      return m_items[m_current_index]->x();
    }
  }();
  auto min_offset = [=] {
  if(m_direction == Qt::Vertical) {
    return std::abs(column_layout->itemAt(0)->geometry().y() - item_pos);
  } else {
    return std::abs(column_layout->itemAt(0)->geometry().x() - item_pos);
  }
  }();
  auto index = 0;
  for(auto i = 2; i < column_layout->count(); i += 2) {
    if(!column_layout->itemAt(i)->widget()) {
      continue;
    }
    auto offset = [=] {
      if(m_direction == Qt::Vertical) {
        return std::abs(column_layout->itemAt(i)->geometry().y() - item_pos);
      } else {
        return std::abs(column_layout->itemAt(i)->geometry().x() - item_pos);
      }
    }();
    if(offset < min_offset) {
      index = i;
      min_offset = offset;
    }
  }
  for(auto i = m_items.begin(); i != m_items.end(); ++i) {
    if(column_layout->itemAt(index)->widget() == *i) {
      update_current(static_cast<int>(std::distance(m_items.begin(), i)));
      break;
    }
  }
}

void ListView::query() {
  if(m_query.isEmpty()) {
    return;
  }
  auto index = (m_current_index + 1) % m_list_model->get_size();
  while(index != m_current_index) {
    if(auto value = m_list_model->get<QString>(index).toUpper();
        value.startsWith(m_query)) {
      update_current(index);
      break;
    }
    index = (index + 1) % m_list_model->get_size();
  }
}
