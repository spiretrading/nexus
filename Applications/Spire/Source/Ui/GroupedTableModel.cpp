#include "Spire/Ui/GroupedTableModel.hpp"
#include <algorithm>
#include <boost/scope/scope_exit.hpp>
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Details/DefaultTableAggregation.hpp"

using namespace boost;
using namespace boost::scope;
using namespace Spire;

namespace {
  struct AggregateCell {
    std::any m_value;
    std::unique_ptr<TableAggregation> m_accumulator;
    bool m_is_dirty = false;
  };
}

struct GroupedTableModel::Group {
  QString m_key;
  std::shared_ptr<Group> m_parent;
  int m_depth;
  int m_row;
  int m_leaf_count;
  int m_descendant_count;
  std::vector<AggregateCell> m_aggregates;

  QString label() const {
    return QStringLiteral("%1 (%2)").arg(m_key, QString::number(m_leaf_count));
  }
};

struct GroupedTableModel::Row {
  std::shared_ptr<Group> m_group;
  TableRowIndexTracker m_source;

  bool is_heading() const {
    return m_source.get_index() == -1;
  }
};

GroupedTableModel::GroupedTableModel(
  std::shared_ptr<TableModel> source, std::vector<int> columns)
  : GroupedTableModel(std::move(source), std::move(columns), {}) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
    std::vector<int> columns, std::vector<TableAggregationFactory> aggregations)
    : m_source(std::move(source)),
      m_columns(std::move(columns)),
      m_aggregations(std::move(aggregations)),
      m_is_source_transaction(false) {
  validate_columns(m_columns);
  if(m_columns.empty()) {
    m_transaction.set_size(m_source->get_row_size());
  } else {
    m_indices.resize(m_source->get_row_size(), -1);
    for(auto i = 0; i != m_source->get_row_size(); ++i) {
      insert(i, 0);
    }
    refresh_aggregates();
  }
  m_connection = m_source->connect_operation_signal(
    std::bind_front(&GroupedTableModel::on_operation, this));
}

GroupedTableModel::~GroupedTableModel() = default;

const std::shared_ptr<TableModel>& GroupedTableModel::get_source() const {
  return m_source;
}

const std::vector<int>& GroupedTableModel::get_grouping() const {
  return m_columns;
}

void GroupedTableModel::set_grouping(std::vector<int> columns) {
  if(columns == m_columns) {
    return;
  }
  validate_columns(columns);
  auto common = std::ranges::mismatch(m_columns, columns);
  auto shared_depth = static_cast<int>(common.in1 - m_columns.begin());
  transact([&] {
    flush_updates(nullptr);
    if(m_columns.empty()) {
      for(auto i = 0; i != m_source->get_row_size(); ++i) {
        m_rows.emplace_back(nullptr, TableRowIndexTracker(i));
        m_indices.push_back(i);
      }
    } else {
      for(auto i = get_row_size() - 1; i >= 0; --i) {
        auto& row = m_rows[i];
        if(!row.is_heading()) {
          row.m_group.reset();
        } else if(row.m_group->m_depth >= shared_depth) {
          for(auto group = row.m_group->m_parent; group;
              group = group->m_parent) {
            --group->m_descendant_count;
          }
          remove_row(i);
        }
      }
    }
    if(columns.empty()) {
      for(auto i = 0; i != m_source->get_row_size(); ++i) {
        move_row(m_indices[i], i);
      }
      m_rows.clear();
      m_indices.clear();
      m_columns.clear();
    } else {
      m_columns = std::move(columns);
      for(auto i = 0; i != m_source->get_row_size(); ++i) {
        insert(i, shared_depth);
      }
    }
    refresh_aggregates();
  });
}

int GroupedTableModel::index_to_source(int row) const {
  if(row < 0 || row >= get_row_size()) {
    return -1;
  } else if(m_columns.empty()) {
    return row;
  }
  return m_rows[row].m_source.get_index();
}

int GroupedTableModel::index_from_source(int row) const {
  if(m_columns.empty()) {
    return index_to_source(row);
  } else if(row < 0 || row >= std::ssize(m_indices)) {
    return -1;
  }
  return m_indices[row];
}

int GroupedTableModel::get_row_size() const {
  if(m_columns.empty()) {
    return m_source->get_row_size();
  }
  return static_cast<int>(m_rows.size());
}

int GroupedTableModel::get_column_size() const {
  return m_source->get_column_size();
}

AnyRef GroupedTableModel::at(int row, int column) const {
  if(m_columns.empty()) {
    return m_source->at(row, column);
  }
  auto& entry = m_rows.at(row);
  if(!entry.is_heading()) {
    return m_source->at(entry.m_source.get_index(), column);
  } else if(column == m_columns.front()) {
    return entry.m_group->label();
  }
  return std::as_const(entry.m_group->m_aggregates.at(column).m_value);
}

QValidator::State GroupedTableModel::set(
    int row, int column, const std::any& value) {
  auto source = index_to_source(row);
  if(source == -1) {
    return QValidator::Invalid;
  }
  return m_source->set(source, column, value);
}

QValidator::State GroupedTableModel::remove(int row) {
  if(row < 0 || row >= get_row_size()) {
    throw std::out_of_range("The row is out of range.");
  }
  auto source = index_to_source(row);
  if(source == -1) {
    return QValidator::Invalid;
  }
  return m_source->remove(source);
}

void GroupedTableModel::validate_columns(
    const std::vector<int>& columns) const {
  auto width = get_column_size();
  auto has_width = width != 0 || m_source->get_row_size() != 0;
  for(auto i = columns.begin(); i != columns.end(); ++i) {
    if(*i < 0 || (has_width && *i >= width) ||
        std::ranges::find(columns.begin(), i, *i) != i) {
      throw std::invalid_argument("Invalid grouping column.");
    }
  }
  if(has_width && std::ssize(m_aggregations) > width) {
    throw std::invalid_argument("Grouping exceeds the source column count.");
  }
}

void GroupedTableModel::index_rows(int first) {
  for(auto i = first; i != get_row_size(); ++i) {
    auto& row = m_rows[i];
    if(row.is_heading()) {
      row.m_group->m_row = i;
    } else {
      m_indices[row.m_source.get_index()] = i;
    }
  }
}

void GroupedTableModel::touch(const std::shared_ptr<Group>& group, int column) {
  auto& cell = group->m_aggregates[column];
  if(!cell.m_is_dirty) {
    cell.m_is_dirty = true;
    m_dirty.emplace_back(group, column);
  }
}

std::unique_ptr<TableAggregation> GroupedTableModel::make_aggregation(
    const Group& group, int column) const {
  if(column < std::ssize(m_aggregations) && m_aggregations[column]) {
    return m_aggregations[column]();
  }
  return Details::make_default_table_aggregation(
    [&, column, this] (const Details::TableValueVisitor& visit) {
      auto end = group.m_row + group.m_descendant_count;
      for(auto row = group.m_row + 1; row <= end; ++row) {
        if(!m_rows[row].is_heading()) {
          visit(m_source->at(m_rows[row].m_source.get_index(), column));
        }
      }
    });
}

void GroupedTableModel::insert(int source, int shared_depth) {
  auto first = 0;
  auto end = get_row_size();
  auto parent = std::shared_ptr<Group>();
  for(auto depth = 0; depth != std::ssize(m_columns); ++depth) {
    auto column = m_columns[depth];
    auto key = to_text(m_source->at(source, column));
    while(first != end && m_rows[first].is_heading() &&
        m_rows[first].m_group->m_key != key) {
      first += 1 + m_rows[first].m_group->m_descendant_count;
    }
    if(first == end || !m_rows[first].is_heading()) {
      auto group = std::make_shared<Group>(std::move(key), parent, depth, first,
        0, 0, std::vector<AggregateCell>(get_column_size()));
      for(auto i = 0; i != get_column_size(); ++i) {
        if(i != m_columns.front()) {
          group->m_aggregates[i].m_accumulator = make_aggregation(*group, i);
        }
      }
      m_rows.emplace(m_rows.begin() + first, group, TableRowIndexTracker(-1));
      index_rows(first + 1);
      for(auto ancestor = parent; ancestor; ancestor = ancestor->m_parent) {
        ++ancestor->m_descendant_count;
      }
      m_transaction.push(AddOperation(first));
    }
    parent = m_rows[first].m_group;
    ++first;
    end = first + parent->m_descendant_count;
  }
  auto position = std::ranges::partition_point(m_rows.begin() + first,
    m_rows.begin() + end, [=] (const Row& row) {
      return row.m_group && row.m_source.get_index() < source;
    });
  auto index = static_cast<int>(position - m_rows.begin());
  auto previous_row = m_indices[source];
  if(previous_row == -1) {
    m_rows.emplace(position, parent, TableRowIndexTracker(source));
    index_rows(index);
    m_transaction.push(AddOperation(index));
  } else {
    m_rows[previous_row].m_group = parent;
    move_row(previous_row, index);
  }
  for(auto group = parent; group; group = group->m_parent) {
    if(previous_row == -1 || group->m_depth >= shared_depth) {
      ++group->m_descendant_count;
    }
    if(group->m_depth < shared_depth) {
      continue;
    }
    auto previous = group->label();
    ++group->m_leaf_count;
    for(auto column = 0; column != get_column_size(); ++column) {
      if(auto& accumulator = group->m_aggregates[column].m_accumulator) {
        accumulator->add(m_source->at(source, column));
        touch(group, column);
      }
    }
    m_transaction.push(UpdateOperation(
      group->m_row, m_columns.front(), std::move(previous), group->label()));
  }
}

void GroupedTableModel::remove_row(int row) {
  m_transaction.push(PreRemoveOperation(row));
  if(m_rows[row].is_heading()) {
    m_rows[row].m_group->m_row = -1;
  } else {
    m_indices[m_rows[row].m_source.get_index()] = -1;
  }
  m_rows.erase(m_rows.begin() + row);
  index_rows(row);
  m_transaction.push(RemoveOperation(row));
}

void GroupedTableModel::move_row(int source, int destination) {
  if(source != destination) {
    move_element(m_rows, source, destination);
    index_rows(std::min(source, destination));
    m_transaction.push(MoveOperation(source, destination));
  }
}

void GroupedTableModel::erase(
    int row, int shared_depth, const Changes& changes) {
  auto group = m_rows[row].m_group;
  auto source = m_rows[row].m_source.get_index();
  remove_row(row);
  auto removed = 1;
  while(group) {
    row = group->m_row;
    group->m_descendant_count -= removed;
    if(group->m_depth < shared_depth) {
      group = group->m_parent;
      continue;
    }
    for(auto column = 0; column != get_column_size(); ++column) {
      auto& accumulator = group->m_aggregates[column].m_accumulator;
      if(accumulator) {
        auto i = changes.find(column);
        if(i != changes.end()) {
          accumulator->remove(i->second);
        } else {
          accumulator->remove(m_source->at(source, column));
        }
        touch(group, column);
      }
    }
    auto previous = group->label();
    --group->m_leaf_count;
    if(group->m_leaf_count == 0) {
      remove_row(row);
      ++removed;
    } else {
      m_transaction.push(UpdateOperation(
        row, m_columns.front(), std::move(previous), group->label()));
    }
    group = group->m_parent;
  }
}

void GroupedTableModel::track(const Operation& operation) {
  for(auto& row : m_rows) {
    row.m_source.update(operation);
  }
  index_rows(0);
}

void GroupedTableModel::flush_updates(const Operation* shift) {
  auto updates = std::exchange(m_updates, {});
  for(auto& [index, changes] : updates) {
    auto source = TableRowIndexTracker(index);
    if(shift) {
      source.update(*shift);
    }
    auto row = m_indices[source.get_index()];
    auto group = m_rows[row].m_group;
    auto depth = static_cast<int>(m_columns.size());
    for(auto i = group; i; i = i->m_parent) {
      auto column = m_columns[i->m_depth];
      if(changes.contains(column) &&
          i->m_key != to_text(m_source->at(source.get_index(), column))) {
        depth = i->m_depth;
      }
    }
    for(auto i = group; i; i = i->m_parent) {
      if(i->m_depth < depth) {
        for(auto& [column, previous] : changes) {
          auto& cell = i->m_aggregates[column];
          auto current = m_source->at(source.get_index(), column);
          if(cell.m_accumulator && !is_equal(previous, current)) {
            cell.m_accumulator->update(previous, current);
            touch(i, column);
          }
        }
      }
    }
    if(depth != static_cast<int>(m_columns.size())) {
      erase(row, depth, changes);
      insert(source.get_index(), depth);
    } else {
      for(auto& [column, previous] : changes) {
        m_transaction.push(UpdateOperation(row, column,
          std::move(previous),
          to_any(m_source->at(source.get_index(), column))));
      }
    }
  }
}

void GroupedTableModel::refresh_aggregates() {
  auto dirty = std::exchange(m_dirty, {});
  for(auto& [group, column] : dirty) {
    auto& cell = group->m_aggregates[column];
    cell.m_is_dirty = false;
    if(group->m_row != -1) {
      auto value = cell.m_accumulator->get();
      if(!is_equal(cell.m_value, value)) {
        auto previous = std::exchange(cell.m_value, std::move(value));
        m_transaction.push(UpdateOperation(
          group->m_row, column, std::move(previous), cell.m_value));
      }
    }
  }
}

void GroupedTableModel::move_leaf(int source) {
  auto row = m_indices[source];
  auto& group = *m_rows[row].m_group;
  auto destination = row;
  while(destination > group.m_row + 1 &&
      m_rows[destination - 1].m_source.get_index() > source) {
    --destination;
  }
  while(destination < group.m_row + group.m_descendant_count &&
      m_rows[destination + 1].m_source.get_index() < source) {
    ++destination;
  }
  move_row(row, destination);
}

void GroupedTableModel::apply(const Operation& operation) {
  visit(operation,
    [&] (const AddOperation& add) {
      if(m_indices.empty()) {
        validate_columns(m_columns);
      }
      m_indices.insert(m_indices.begin() + add.m_index, -1);
      if(add.m_index + 1 != m_source->get_row_size()) {
        track(add);
      }
      flush_updates(&operation);
      insert(add.m_index, 0);
    },
    [&] (const PreRemoveOperation& removal) {
      flush_updates(nullptr);
      erase(m_indices[removal.m_index], 0, {});
    },
    [&] (const RemoveOperation& removal) {
      m_indices.erase(m_indices.begin() + removal.m_index);
      track(PreRemoveOperation(removal.m_index));
    },
    [&] (const MoveOperation& move) {
      track(move);
      move_leaf(move.m_destination);
      flush_updates(&operation);
    },
    [&] (const UpdateOperation& update) {
      m_updates[update.m_row].try_emplace(update.m_column, update.m_previous);
    });
}

void GroupedTableModel::on_operation(const Operation& operation) {
  visit(operation,
    [&] (const StartTransaction&) {
      m_is_source_transaction = true;
      start_transaction();
    },
    [&] (const EndTransaction&) {
      m_is_source_transaction = false;
      auto end = scope_exit([&] {
        end_transaction();
      });
      flush_updates(nullptr);
      refresh_aggregates();
    },
    [&] (const auto&) {
      if(m_columns.empty()) {
        m_transaction.push(operation);
      } else if(m_is_source_transaction) {
        apply(operation);
      } else {
        transact([&] {
          apply(operation);
          flush_updates(nullptr);
          refresh_aggregates();
        });
      }
    });
}
