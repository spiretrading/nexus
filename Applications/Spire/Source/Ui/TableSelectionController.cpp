#include "Spire/Ui/TableSelectionController.hpp"
#include <boost/signals2/shared_connection_block.hpp>

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

  auto increment(auto i, int direction, int column_size) {
    advance(i, direction, column_size);
    return i;
  }

  int find_index(auto value, auto& list) {
    for(auto i = 0; i != list.get_size(); ++i) {
      if(list.get(i) == value) {
        return i;
      }
    }
    return -1;
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
  ++m_row_size;
  auto item_blocker = shared_connection_block(m_item_connection);
  auto row_blocker = shared_connection_block(m_row_connection);
  m_selection->transact([&] {
    auto& rows = m_selection->get_row_selection();
    for(auto i = 0; i != rows->get_size(); ++i) {
      auto selection = rows->get(i);
      if(selection >= index) {
        rows->set(i, selection + 1);
      }
    }
    auto& items = m_selection->get_item_selection();
    for(auto i = 0; i != items->get_size(); ++i) {
      auto& selection = items->get(i);
      if(selection.m_row >= index) {
        auto update = selection;
        ++update.m_row;
        items->set(i, update);
      }
    }
  });
}

void TableSelectionController::remove_row(int index) {
  --m_row_size;
  auto row_operation = optional<ListModel<int>::Operation>();
  auto item_operation = optional<ListModel<TableIndex>::Operation>();
  {
    auto item_blocker = shared_connection_block(m_item_connection);
    auto row_blocker = shared_connection_block(m_row_connection);
    m_selection->transact([&] {
      {
        auto& rows = m_selection->get_row_selection();
        auto i = 0;
        while(i != rows->get_size()) {
          auto selection = rows->get(i);
          if(selection == index) {
            if(rows->remove(i) != QValidator::State::Invalid) {
              row_operation = ListModel<int>::RemoveOperation(i, index);
            } else {
              ++i;
            }
          } else {
            if(selection > index) {
              rows->set(i, selection - 1);
            }
            ++i;
          }
        }
      }
      {
        auto& items = m_selection->get_item_selection();
        auto i = 0;
        while(i != items->get_size()) {
          auto selection = items->get(i);
          if(selection.m_row == index) {
            if(items->remove(i) != QValidator::State::Invalid) {
              item_operation =
                ListModel<TableIndex>::RemoveOperation(i, selection);
            } else {
              ++i;
            }
          } else {
            if(selection.m_row > index) {
              auto update = selection;
              --update.m_row;
              items->set(i, update);
            }
            ++i;
          }
        }
      }
    });
  }
  if(row_operation) {
    m_row_operation_signal(*row_operation);
  }
  if(item_operation) {
    m_item_operation_signal(*item_operation);
  }
}

void TableSelectionController::move_row(int source, int destination) {
  auto item_blocker = shared_connection_block(m_item_connection);
  auto row_blocker = shared_connection_block(m_row_connection);
  auto direction = get_direction(destination, source);
  m_selection->transact([&] {
    auto& rows = m_selection->get_row_selection();
    for(auto i = 0; i != rows->get_size(); ++i) {
      auto selection = rows->get(i);
      if(selection >= source || selection <= destination) {
        rows->set(i, selection + direction);
      }
    }
    auto& items = m_selection->get_item_selection();
    for(auto i = 0; i != items->get_size(); ++i) {
      auto selection = items->get(i);
      if(selection.m_row >= source || selection.m_row <= destination) {
        selection.m_row += direction;
        items->set(i, selection);
      }
    }
  });
}

void TableSelectionController::select_all() {
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
    } else if(!m_current) {
      auto select = [&] (auto selection, auto index, auto anchor) {
        auto direction = get_direction(anchor, index);
        for(auto i = anchor; can_advance(i, direction, index);
            advance(i, direction, m_column_size)) {
          selection->push(i);
        }
      };
      m_selection->transact([&] {
        select(
          m_selection->get_row_selection(), index.m_row, m_range_anchor->m_row);
        select(m_selection->get_column_selection(), index.m_column,
          m_range_anchor->m_column);
        select(m_selection->get_item_selection(), index, *m_range_anchor);
      });
    } else {
      auto select =
        [&] (auto selection, auto current, auto index, auto anchor) {
          auto direction = get_direction(current, index);
          for(auto i = current; can_advance(i, direction, index);
              advance(i, direction, m_column_size)) {
            if(i == anchor || !can_advance(i, direction, anchor)) {
              if(find_index(i, *selection) == -1) {
                selection->push(i);
              }
            } else if(i != index) {
              auto j = find_index(i, *selection);
              if(j != -1) {
                selection->remove(j);
              }
            }
          }
        };
      m_selection->transact([&] {
        select(m_selection->get_row_selection(), m_current->m_row, index.m_row,
          m_range_anchor->m_row);
        select(m_selection->get_column_selection(), m_current->m_column,
          index.m_column, m_range_anchor->m_column);
        select(m_selection->get_item_selection(), *m_current, index,
          *m_range_anchor);
      });
    }
  }
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

void TableSelectionController::on_item_operation(
    const ListModel<Index>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::ItemSelectionModel::RemoveOperation&
        operation) {
      if(operation.get_value() == m_range_anchor) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::ItemSelectionModel::UpdateOperation&
        operation) {
      if(operation.get_previous() == m_range_anchor) {
        m_range_anchor = none;
      }
    });
  m_item_operation_signal(operation);
}

void TableSelectionController::on_row_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::RowSelectionModel::RemoveOperation&
        operation) {
      if(m_range_anchor && operation.get_value() == m_range_anchor->m_row) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::RowSelectionModel::UpdateOperation&
        operation) {
      if(m_range_anchor && operation.get_previous() == m_range_anchor->m_row) {
        m_range_anchor = none;
      }
    });
  m_row_operation_signal(operation);
}

void TableSelectionController::on_column_operation(
    const ListModel<int>::Operation& operation) {
  visit(operation,
    [&] (const TableSelectionModel::ColumnSelectionModel::RemoveOperation&
        operation) {
      if(m_range_anchor && operation.get_value() == m_range_anchor->m_column) {
        m_range_anchor = none;
      }
    },
    [&] (const TableSelectionModel::ColumnSelectionModel::UpdateOperation&
        operation) {
      if(m_range_anchor &&
          operation.get_previous() == m_range_anchor->m_column) {
        m_range_anchor = none;
      }
    });
  m_column_operation_signal(operation);
}
