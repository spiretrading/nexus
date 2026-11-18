#include "Spire/Ui/TableSelectionController.hpp"
#include <unordered_set>
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  int get_direction(int start, int end) {
    if(start <= end) {
      return 1;
    }
    return -1;
  }

  int get_direction(TableIndex start, TableIndex end) {
    if(start.m_row < end.m_row) {
      return 1;
    } else if(start.m_row == end.m_row) {
      return get_direction(start.m_column, end.m_column);
    }
    return -1;
  }

  bool can_advance(int i, int direction, int index) {
    return direction * i <= direction * index;
  }

  bool can_advance(TableIndex i, int direction, TableIndex index) {
    if(direction * i.m_row < direction * index.m_row) {
      return true;
    } else if(i.m_row == index.m_row) {
      return can_advance(i.m_column, direction, index.m_column);
    }
    return false;
  }

  void advance(int& i, int direction, int column_size) {
    i += direction;
  }

  void advance(TableIndex& i, int direction, int column_size) {
    if(direction == 1) {
      if(i.m_column == column_size - 1) {
        i.m_column = 0;
        ++i.m_row;
      } else {
        ++i.m_column;
      }
    } else {
      if(i.m_column == 0) {
        i.m_column = column_size - 1;
        --i.m_row;
      } else {
        --i.m_column;
      }
    }
  }

  bool is_in_range(auto value, auto anchor, auto end) {
    auto direction = get_direction(anchor, end);
    return can_advance(anchor, direction, value) &&
      can_advance(value, direction, end);
  }

  void reconcile_range(
      auto& list, auto start, auto end, int columns, auto is_old) {
    auto kept = std::unordered_set<decltype(start)>();
    for(auto i = list.get_size(); i-- > 0;) {
      auto value = list.get(i);
      if(is_in_range(value, start, end)) {
        kept.insert(value);
      } else if(is_old(value)) {
        list.remove(i);
      }
    }
    auto direction = get_direction(start, end);
    for(auto i = start; can_advance(i, direction, end);
        advance(i, direction, columns)) {
      if(!kept.contains(i)) {
        list.push(i);
      }
    }
  }

  auto find_index(auto value, auto& list) {
    auto i = std::find(list.begin(), list.end(), value);
    if(i == list.end()) {
      return std::ptrdiff_t(-1);
    }
    return std::distance(list.begin(), i);
  }
}

TableSelectionController::TableSelectionController(
  std::shared_ptr<SelectionModel> selection, int row_size, int column_size)
  : m_mode(Mode::SINGLE),
    m_selection(std::move(selection)),
    m_row_size(row_size),
    m_column_size(column_size),
    m_item_connection(
      m_selection->get_item_selection()->connect_operation_signal(
        std::bind_front(&TableSelectionController::on_item_operation, this))),
    m_row_connection(m_selection->get_row_selection()->connect_operation_signal(
      std::bind_front(&TableSelectionController::on_row_operation, this))),
    m_column_connection(
      m_selection->get_column_selection()->connect_operation_signal(
        std::bind_front(
          &TableSelectionController::on_column_operation, this))) {}

const std::shared_ptr<TableSelectionController::SelectionModel>&
    TableSelectionController::get_selection() const {
  return m_selection;
}

TableSelectionController::Mode TableSelectionController::get_mode() const {
  return m_mode;
}

void TableSelectionController::set_mode(Mode mode) {
  m_mode = mode;
}

int TableSelectionController::get_row_size() const {
  return m_row_size;
}

int TableSelectionController::get_column_size() const {
  return m_column_size;
}

void TableSelectionController::add_row(int index) {
  save_range([=] (int first, int last) {
    return first < index && index <= last;
  });
  if(m_previous_range) {
    m_previous_range->emplace(m_previous_range->begin() + index, 0, 0);
  }
  ++m_row_size;
  auto item_blocker = shared_connection_block(m_item_connection);
  auto row_blocker = shared_connection_block(m_row_connection);
  update_rows(TableModel::AddOperation(index));
}

void TableSelectionController::remove_row(int index) {
  if(m_previous_range) {
    m_previous_range->erase(m_previous_range->begin() + index);
  }
  update_rows(TableModel::PreRemoveOperation(index));
  --m_row_size;
}

void TableSelectionController::move_row(int source, int destination) {
  save_range([=] (int first, int last) {
    return std::min(source, destination) <= last &&
      first <= std::max(source, destination);
  });
  if(m_previous_range) {
    move_element(*m_previous_range, source, destination);
  }
  auto item_blocker = shared_connection_block(m_item_connection);
  auto row_blocker = shared_connection_block(m_row_connection);
  update_rows(TableModel::MoveOperation(source, destination));
}

void TableSelectionController::select_all() {
  if(!m_selection->is_active()) {
    return;
  }
  auto select = [&] (auto selection, auto current, auto start, auto end) {
    for(auto i = start; i != end; advance(i, 1, m_column_size)) {
      if(i != current && find_index(i, *selection) == -1) {
        selection->push(i);
      }
    }
    if(current) {
      if(find_index(*current, *selection) == -1) {
        selection->push(*current);
      }
    }
  };
  m_selection->transact([&] {
    auto [current_row, current_column, current] = [&] {
      if(m_current) {
        return std::tuple(optional<int>(m_current->m_row),
          optional<int>(m_current->m_column), optional<TableIndex>(m_current));
      } else {
        return std::tuple(
          optional<int>(), optional<int>(), optional<TableIndex>());
      }
    }();
    select(m_selection->get_row_selection(), current_row, 0, m_row_size);
    select(
      m_selection->get_column_selection(), current_column, 0, m_column_size);
    select(m_selection->get_item_selection(), current, TableIndex(0, 0),
      TableIndex(m_row_size, 0));
  });
}

void TableSelectionController::click(Index index) {
  if(!m_selection->is_active()) {
    return;
  }
  if(m_mode == Mode::SINGLE) {
    auto select = [&] (auto selection, auto index) {
      if(selection->get_size() != 1 || selection->get(0) != index) {
        clear(*selection);
        selection->push(index);
      }
    };
    m_selection->transact([&] {
      select(m_selection->get_row_selection(), index.m_row);
      select(m_selection->get_column_selection(), index.m_column);
      select(m_selection->get_item_selection(), index);
    });
    m_range_anchor = index;
  } else if(m_mode == Mode::INCREMENTAL) {
    auto select = [&] (auto selection, auto index) {
      auto selection_index = find_index(index, *selection);
      if(selection_index == -1) {
        selection->push(index);
      } else {
        selection->remove(selection_index);
      }
    };
    m_selection->transact([&] {
      select(m_selection->get_row_selection(), index.m_row);
      select(m_selection->get_column_selection(), index.m_column);
      select(m_selection->get_item_selection(), index);
    });
    m_range_anchor = index;
  } else if(m_mode == Mode::RANGE) {
    if(!m_range_anchor) {
      m_mode = Mode::SINGLE;
      click(index);
      m_mode = Mode::RANGE;
      m_range_anchor = index;
    } else {
      auto select =
        [&] (auto& selection, auto current, auto index, auto anchor) {
          auto direction = get_direction(current, index);
          for(auto i = current; can_advance(i, direction, index);
              advance(i, direction, m_column_size)) {
            if(i == anchor || !can_advance(i, direction, anchor)) {
              if(find_index(i, selection) == -1) {
                selection.push(i);
              }
            } else if(i != index) {
              auto j = find_index(i, selection);
              if(j != -1) {
                selection.remove(j);
              }
            }
          }
        };
      auto anchor = *m_range_anchor;
      auto current = m_current.get_value_or(anchor);
      m_selection->transact([&] {
        auto& rows = *m_selection->get_row_selection();
        auto& columns = *m_selection->get_column_selection();
        auto& items = *m_selection->get_item_selection();
        if(m_previous_range) {
          reconcile_range(rows, anchor.m_row, index.m_row, m_column_size,
            [&] (int row) {
              return row >= 0 && row < std::ssize(*m_previous_range) &&
                (*m_previous_range)[row].m_end != 0;
            });
          select(columns, current.m_column, index.m_column, anchor.m_column);
          reconcile_range(items, anchor, index, m_column_size,
            [&] (Index cell) {
              if(cell.m_row < 0 ||
                  cell.m_row >= std::ssize(*m_previous_range)) {
                return false;
              }
              auto& range = (*m_previous_range)[cell.m_row];
              return cell.m_column >= range.m_begin &&
                cell.m_column < range.m_end;
            });
        } else {
          select(rows, current.m_row, index.m_row, anchor.m_row);
          select(columns, current.m_column, index.m_column, anchor.m_column);
          select(items, current, index, anchor);
        }
      });
    }
  }
  m_previous_range = none;
  m_current = index;
}

void TableSelectionController::navigate(Index index) {
  if(m_mode == Mode::SINGLE || m_mode == Mode::RANGE) {
    click(index);
  }
}

connection TableSelectionController::connect_item_operation_signal(
    const ListModel<Index>::OperationSignal::slot_type& slot) const {
  return m_item_operation_signal.connect(slot);
}

connection TableSelectionController::connect_row_operation_signal(
    const ListModel<int>::OperationSignal::slot_type& slot) const {
  return m_row_operation_signal.connect(slot);
}

connection TableSelectionController::connect_column_operation_signal(
    const ListModel<int>::OperationSignal::slot_type& slot) const {
  return m_column_operation_signal.connect(slot);
}

void TableSelectionController::save_range(auto is_affected) {
  if(m_previous_range || !m_range_anchor || !m_current ||
      m_range_anchor->m_row == m_current->m_row) {
    return;
  }
  auto [first, last] = std::minmax(*m_range_anchor, *m_current,
    [] (auto left, auto right) { return left.m_row < right.m_row; });
  if(!is_affected(first.m_row, last.m_row)) {
    return;
  }
  m_previous_range.emplace(m_row_size);
  auto& range = *m_previous_range;
  std::fill(range.begin() + first.m_row, range.begin() + last.m_row + 1,
    ColumnRange(0, m_column_size));
  range[first.m_row].m_begin = first.m_column;
  range[last.m_row].m_end = last.m_column + 1;
}

void TableSelectionController::update_rows(
    const TableModel::Operation& operation) {
  auto update = [&] (optional<Index>& index) {
    if(index) {
      auto tracker = TableRowIndexTracker(index->m_row);
      tracker.update(operation);
      if(tracker.get_index() == -1) {
        index = none;
      } else {
        index->m_row = tracker.get_index();
      }
    }
  };
  update(m_range_anchor);
  update(m_current);
  m_selection->update_rows(operation);
}

void TableSelectionController::on_item_operation(
    const ListModel<Index>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::ItemSelectionModel::PreRemoveOperation&
        operation) {
      m_item_operation_signal(operation);
      if(!m_selection->m_is_updating &&
          m_selection->get_item_selection()->get(operation.m_index) ==
            m_range_anchor) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::ItemSelectionModel::UpdateOperation&
        operation) {
      if(!m_selection->m_is_updating &&
          operation.get_previous() == m_range_anchor) {
        m_range_anchor = none;
      }
      m_item_operation_signal(operation);
    },
    [&] (const auto& operation) {
      m_item_operation_signal(operation);
    });
}

void TableSelectionController::on_row_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::RowSelectionModel::PreRemoveOperation&
        operation) {
      m_row_operation_signal(operation);
      if(!m_selection->m_is_updating && m_range_anchor &&
          m_selection->get_row_selection()->get(operation.m_index) ==
            m_range_anchor->m_row) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::RowSelectionModel::UpdateOperation&
        operation) {
      if(!m_selection->m_is_updating && m_range_anchor &&
          operation.get_previous() == m_range_anchor->m_row) {
        m_range_anchor = none;
      }
      m_row_operation_signal(operation);
    },
    [&] (const auto& operation) {
      m_row_operation_signal(operation);
    });
}

void TableSelectionController::on_column_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::ColumnSelectionModel::PreRemoveOperation&
        operation) {
      m_column_operation_signal(operation);
      if(m_range_anchor &&
          m_selection->get_column_selection()->get(operation.m_index) ==
            m_range_anchor->m_column) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::ColumnSelectionModel::UpdateOperation&
        operation) {
      if(m_range_anchor &&
          operation.get_previous() == m_range_anchor->m_column) {
        m_range_anchor = none;
      }
      m_column_operation_signal(operation);
    },
    [&] (const auto& operation) {
      m_column_operation_signal(operation);
    });
}
