#include "Spire/Ui/GroupedTableModel.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ranked_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/scope/scope_exit.hpp>
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DefaultTableAggregator.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"

using namespace boost;
using namespace boost::scope;
using namespace Spire;

namespace {
  bool is_nan_key(const AnyRef& key) {
    if(auto number = any_cast<float>(&key)) {
      return std::isnan(*number);
    } else if(auto number = any_cast<double>(&key)) {
      return std::isnan(*number);
    } else if(auto number = any_cast<long double>(&key)) {
      return std::isnan(*number);
    }
    return false;
  }

  bool equal_keys(const std::any& left, const AnyRef& right) {
    if(left.type() != right.get_type()) {
      return false;
    } else if(!left.has_value()) {
      return true;
    }
    return Spire::is_equal(left, right) ||
      is_nan_key(right) && is_nan_key(left);
  }

  std::optional<std::size_t> group_hash(const AnyRef& key, const void* parent) {
    auto hash = [&] () -> std::optional<std::size_t> {
      if(!key.has_value() || is_nan_key(key)) {
        return 0;
      }
      return hash_value(key);
    }();
    if(hash) {
      hash_combine(*hash, parent);
    }
    return hash;
  }

  bool compare_siblings(const AnyRef& left, int left_row, const AnyRef& right,
      int right_row, int column) {
    if(!left.has_value() || !right.has_value()) {
      return !left.has_value() && right.has_value();
    }
    return compare(left, right);
  }

  struct AggregateCell {
    std::any m_value;
    std::unique_ptr<TableAggregator> m_accumulator;
    bool m_is_dirty = false;
  };
}

struct GroupedTableModel::Group {
  std::any m_key;
  QString m_name;
  std::shared_ptr<Group> m_parent;
  std::size_t m_sequence;
  int m_depth;
  int m_row;
  int m_leaf_count;
  int m_descendant_count;
  std::vector<AggregateCell> m_aggregates;

  QString label() const {
    return QStringLiteral("%1 (%2)").arg(m_name, QString::number(m_leaf_count));
  }
};

struct GroupedTableModel::Row {
  std::shared_ptr<Group> m_group;
  TableRowIndexTracker m_source;

  bool is_heading() const {
    return m_source.get_index() == -1;
  }
};

struct GroupedTableModel::PendingRows {
  std::vector<Row> m_rows;
  multi_index_container<int, multi_index::indexed_by<
    multi_index::ranked_unique<multi_index::identity<int>>>> m_order;
  int m_placed = 0;
};

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source)
  : GroupedTableModel(std::move(source), {}) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
  std::vector<AggregatorFactory> aggregators)
  : GroupedTableModel(
      std::move(source), std::move(aggregators), Comparator()) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
  std::vector<AggregatorFactory> aggregators, ValueComparator comparator)
  : GroupedTableModel(std::move(source), std::move(aggregators),
      [comparator = std::move(comparator)] (
          const AnyRef& left, int, const AnyRef& right, int, int) {
        return comparator(left, right);
      }) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
  std::vector<AggregatorFactory> aggregators, Comparator comparator)
  : GroupedTableModel(std::move(source), std::move(aggregators),
      std::move(comparator), {}) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
    std::vector<AggregatorFactory> aggregators, Comparator comparator,
    AggregateEquality is_equal)
    : m_source(std::move(source)),
      m_has_matching_columns(true),
      m_aggregators(std::move(aggregators)),
      m_comparator(std::move(comparator)),
      m_is_equal(std::move(is_equal)),
      m_next_group(0),
      m_pending_rows(nullptr),
      m_is_source_transaction(false) {
  if(!m_comparator) {
    m_comparator = &compare_siblings;
  }
  if(!m_is_equal) {
    m_is_equal =
      static_cast<bool (*)(const AnyRef&, const AnyRef&)>(&Spire::is_equal);
  }
  validate_columns(m_columns);
  m_transaction.set_size(m_source->get_row_size());
  m_connection = m_source->connect_operation_signal(
    std::bind_front(&GroupedTableModel::on_operation, this));
}

GroupedTableModel::~GroupedTableModel() = default;

const std::shared_ptr<TableModel>& GroupedTableModel::get_source() const {
  return m_source;
}

const GroupedTableModel::Comparator& GroupedTableModel::get_comparator() const {
  return m_comparator;
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
      group_leaves(std::move(columns));
    } else {
      for(auto i = get_row_size() - 1; i >= 0; --i) {
        auto& row = m_rows[i];
        if(!row.is_heading()) {
          while(row.m_group && row.m_group->m_depth >= shared_depth) {
            row.m_group = row.m_group->m_parent;
          }
        } else if(row.m_group->m_depth >= shared_depth) {
          for(auto group = row.m_group->m_parent; group;
              group = group->m_parent) {
            --group->m_descendant_count;
          }
          remove_row(i);
        }
      }
      if(columns.empty()) {
        if(m_order.empty()) {
          restore_source_order();
        }
        m_columns.clear();
      } else {
        m_columns = std::move(columns);
        for(auto i = 0; i != get_source_row_size(); ++i) {
          insert(i, m_rows[m_indices[i]].m_group);
        }
      }
    }
    refresh_aggregates();
    if(!m_order.empty()) {
      sort_all();
    }
    m_grouping_signal(m_columns);
  });
}

void GroupedTableModel::set_column_order(
    std::vector<SortedTableModel::ColumnOrder> order) {
  std::erase_if(order, [] (const auto& column) {
    return column.m_order == SortedTableModel::Ordering::NONE;
  });
  auto key = [] (const auto& column) {
    return std::pair(column.m_index, column.m_order);
  };
  if(std::ranges::equal(order, m_order, {}, key, key)) {
    return;
  }
  auto columns = std::vector<int>();
  for(auto& column : order) {
    columns.push_back(column.m_index);
  }
  validate_columns(columns);
  transact([&] {
    flush_updates(nullptr);
    refresh_aggregates();
    if(is_identity()) {
      materialize_rows();
    }
    if(order.empty() && m_columns.empty()) {
      restore_source_order();
      m_order.clear();
      m_unsorted.clear();
    } else {
      m_order = std::move(order);
      sort_all();
    }
  });
}

boost::signals2::connection GroupedTableModel::connect_grouping_signal(
    const GroupingSignal::slot_type& slot) const {
  return m_grouping_signal.connect(slot);
}

int GroupedTableModel::get_parent(int row) const {
  if(m_columns.empty()) {
    return -1;
  }
  auto& entry = get_row(row);
  auto group = entry.m_group.get();
  if(entry.is_heading()) {
    group = group->m_parent.get();
  }
  if(group) {
    return group->m_row;
  }
  return -1;
}

QString GroupedTableModel::get_group_name(int row) const {
  if(index_to_source(row) != -1) {
    return {};
  }
  return m_rows.at(row).m_group->m_name;
}

int GroupedTableModel::get_group_size(int row) const {
  if(index_to_source(row) != -1) {
    return 0;
  }
  return m_rows.at(row).m_group->m_leaf_count;
}

int GroupedTableModel::get_descendant_count(int row) const {
  if(index_to_source(row) != -1) {
    return 0;
  }
  return m_rows.at(row).m_group->m_descendant_count;
}

int GroupedTableModel::get_first_leaf(int row) const {
  if(row < 0 || row >= get_row_size()) {
    return -1;
  }
  while(row + 1 < get_row_size() && get_parent(row + 1) == row) {
    ++row;
    if(!m_rows[row].is_heading()) {
      return row;
    }
  }
  return -1;
}

int GroupedTableModel::index_to_source(int row) const {
  if(row < 0 || row >= get_row_size()) {
    return -1;
  } else if(is_identity()) {
    return row;
  }
  return get_row(row).m_source.get_index();
}

int GroupedTableModel::index_from_source(int row) const {
  if(is_identity()) {
    return index_to_source(row);
  } else if(row < 0 || row >= std::ssize(m_indices)) {
    return -1;
  }
  auto index = m_indices[row];
  if(m_pending_rows && index < 0) {
    return static_cast<int>(m_pending_rows->m_placed +
      m_pending_rows->m_order.lower_bound_rank(-index - 1));
  }
  return index;
}

int GroupedTableModel::get_row_size() const {
  if(is_identity()) {
    return get_source_row_size();
  }
  if(m_pending_rows) {
    return static_cast<int>(
      m_pending_rows->m_placed + m_pending_rows->m_order.size());
  }
  return static_cast<int>(m_rows.size());
}

int GroupedTableModel::get_column_size() const {
  if(m_column_count) {
    return *m_column_count;
  }
  return m_source->get_column_size();
}

AnyRef GroupedTableModel::at(int row, int column) const {
  if(is_identity()) {
    if(!m_has_matching_columns) {
      throw std::out_of_range("The row is out of range.");
    }
    return m_source->at(row, column);
  }
  auto& entry = get_row(row);
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

int GroupedTableModel::get_source_row_size() const {
  if(!m_has_matching_columns) {
    return 0;
  }
  return m_source->get_row_size();
}

void GroupedTableModel::set_column_count(int count) {
  m_column_count = count;
  validate_columns(m_columns);
}

void GroupedTableModel::validate_columns(
    const std::vector<int>& columns) const {
  auto width = get_column_size();
  auto has_width =
    m_column_count || width != 0 || m_source->get_row_size() != 0;
  for(auto i = columns.begin(); i != columns.end(); ++i) {
    if(*i < 0 || (has_width && *i >= width) ||
        std::ranges::find(columns.begin(), i, *i) != i) {
      throw std::invalid_argument("Invalid column index.");
    }
  }
  if(has_width && std::ssize(m_aggregators) > width) {
    throw std::invalid_argument("Grouping exceeds the source column count.");
  }
}

bool GroupedTableModel::is_identity() const {
  return m_columns.empty() && m_order.empty();
}

const GroupedTableModel::Row& GroupedTableModel::get_row(int index) const {
  if(m_pending_rows && index >= m_pending_rows->m_placed) {
    auto i = m_pending_rows->m_order.nth(index - m_pending_rows->m_placed);
    if(i == m_pending_rows->m_order.end()) {
      throw std::out_of_range("The row is out of range.");
    }
    return m_pending_rows->m_rows[*i];
  }
  return m_rows.at(index);
}

void GroupedTableModel::materialize_rows() {
  for(auto i = 0; i != get_source_row_size(); ++i) {
    m_rows.emplace_back(nullptr, TableRowIndexTracker(i));
    m_indices.push_back(i);
  }
}

void GroupedTableModel::group_leaves(std::vector<int> columns) {
  auto pending = PendingRows();
  for(auto i = 0; i != get_row_size(); ++i) {
    pending.m_order.insert(pending.m_order.end(), i);
  }
  auto rows = make_grouped_rows(columns);
  if(is_identity()) {
    materialize_rows();
  }
  pending.m_rows.swap(m_rows);
  m_rows = std::move(rows);
  for(auto i = 0; i != std::ssize(pending.m_rows); ++i) {
    m_indices[pending.m_rows[i].m_source.get_index()] = -i - 1;
  }
  m_pending_rows = &pending;
  auto restore = scope_exit([&] {
    m_pending_rows = nullptr;
  });
  m_columns = std::move(columns);
  auto heading = -1;
  for(auto& entry : m_rows) {
    auto source = entry.m_source.get_index();
    auto previous = pending.m_order.end();
    if(!entry.is_heading()) {
      previous = pending.m_order.find(-m_indices[source] - 1);
    }
    if(heading != -1 &&
        (entry.is_heading() || previous != pending.m_order.begin())) {
      m_transaction.push(AddOperation(heading));
      heading = -1;
    }
    auto row = pending.m_placed;
    ++pending.m_placed;
    auto group = entry.m_group.get();
    if(entry.is_heading()) {
      group->m_row = row;
      group->m_descendant_count = 0;
      group = group->m_parent.get();
    }
    for(; group; group = group->m_parent.get()) {
      ++group->m_descendant_count;
    }
    if(entry.is_heading()) {
      heading = row;
    } else {
      auto from = row + static_cast<int>(pending.m_order.rank(previous));
      pending.m_order.erase(previous);
      m_indices[source] = row;
      if(from != row) {
        m_transaction.push(MoveOperation(from, row));
      }
    }
  }
  if(heading != -1) {
    m_transaction.push(AddOperation(heading));
  }
}

std::vector<GroupedTableModel::Row> GroupedTableModel::make_grouped_rows(
    const std::vector<int>& columns) {
  auto sources = std::vector<int>(get_source_row_size());
  std::iota(sources.begin(), sources.end(), 0);
  auto rows = std::vector<Row>();
  try {
    append_groups(rows, sources, columns, nullptr, 0);
  } catch(...) {
    m_groups.clear();
    throw;
  }
  return rows;
}

void GroupedTableModel::append_groups(std::vector<Row>& rows,
    const std::vector<int>& sources, const std::vector<int>& columns,
    const std::shared_ptr<Group>& parent, int depth) {
  struct Bucket {
    std::shared_ptr<Group> m_group;
    std::vector<int> m_sources;
  };
  auto buckets = std::vector<Bucket>();
  auto sequence = m_next_group;
  for(auto source : sources) {
    auto key = m_source->at(source, columns[depth]);
    auto hash = group_hash(key, parent.get());
    auto group = find_group(key, parent.get(), hash);
    if(!group) {
      auto& bucket = buckets.emplace_back();
      bucket.m_group =
        make_group(key, to_text(key), parent, depth, columns.front());
      group = bucket.m_group.get();
      if(hash) {
        m_groups.emplace(*hash, group);
      }
    }
    buckets[group->m_sequence - sequence].m_sources.push_back(source);
  }
  for(auto& bucket : buckets) {
    auto& group = bucket.m_group;
    auto start = rows.size();
    rows.emplace_back(group, TableRowIndexTracker(-1));
    group->m_leaf_count = static_cast<int>(bucket.m_sources.size());
    initialize_aggregates(*group, bucket.m_sources);
    if(depth + 1 < std::ssize(columns)) {
      append_groups(rows, bucket.m_sources, columns, group, depth + 1);
    } else {
      for(auto source : bucket.m_sources) {
        rows.emplace_back(group, TableRowIndexTracker(source));
      }
    }
    group->m_descendant_count = static_cast<int>(rows.size() - start - 1);
  }
}

void GroupedTableModel::restore_source_order() {
  for(auto i = 0; i != get_source_row_size(); ++i) {
    move_row(m_indices[i], i);
  }
  m_rows.clear();
  m_indices.clear();
}

void GroupedTableModel::index_rows(int first, int end) {
  for(auto i = first; i != end; ++i) {
    auto& row = m_rows[i];
    if(row.is_heading()) {
      row.m_group->m_row = i;
    } else {
      m_indices[row.m_source.get_index()] = i;
    }
  }
}

bool GroupedTableModel::is_sorted(int column) const {
  return std::ranges::contains(
    m_order, column, &SortedTableModel::ColumnOrder::m_index);
}

AnyRef GroupedTableModel::sort_value(const Row& row, int column) const {
  if(!row.is_heading()) {
    return m_source->at(row.m_source.get_index(), column);
  }
  auto& group = *row.m_group;
  if(column == m_columns.front() || column == m_columns[group.m_depth]) {
    auto leaf = get_first_leaf(group.m_row);
    return m_source->at(
      m_rows[leaf].m_source.get_index(), m_columns[group.m_depth]);
  }
  auto& value = group.m_aggregates[column].m_value;
  if(auto count = std::any_cast<DistinctCount>(&value)) {
    return count->m_value;
  }
  return value;
}

void GroupedTableModel::sort_siblings(const std::shared_ptr<Group>& parent) {
  auto first = 0;
  auto end = get_row_size();
  if(parent) {
    first = parent->m_row + 1;
    end = first + parent->m_descendant_count;
  }
  struct Sibling {
    Row m_row;
    int m_index;
    std::vector<AnyRef> m_keys;
  };
  auto siblings = std::vector<Sibling>();
  for(auto i = first; i < end; ++i) {
    auto& row = m_rows[i];
    auto index = i;
    if(row.is_heading()) {
      i += row.m_group->m_descendant_count;
    }
    if(siblings.empty() && i + 1 == end) {
      return;
    }
    auto& sibling = siblings.emplace_back(row, index, std::vector<AnyRef>());
    for(auto& column : m_order) {
      sibling.m_keys.push_back(sort_value(row, column.m_index));
    }
  }
  std::ranges::sort(siblings, [&] (const Sibling& left, const Sibling& right) {
    for(auto i = 0; i != std::ssize(m_order); ++i) {
      auto& a = left.m_keys[i];
      auto& b = right.m_keys[i];
      auto column = m_order[i].m_index;
      if(left.m_row.is_heading() && column == m_columns.front()) {
        column = m_columns[left.m_row.m_group->m_depth];
      }
      if(m_comparator(a, left.m_index, b, right.m_index, column)) {
        return m_order[i].m_order == SortedTableModel::Ordering::ASCENDING;
      } else if(m_comparator(b, right.m_index, a, left.m_index, column)) {
        return m_order[i].m_order == SortedTableModel::Ordering::DESCENDING;
      }
    }
    if(left.m_row.is_heading()) {
      return left.m_row.m_group->m_sequence < right.m_row.m_group->m_sequence;
    }
    return left.m_row.m_source.get_index() < right.m_row.m_source.get_index();
  });
  for(auto& sibling : siblings) {
    auto& row = sibling.m_row;
    auto count = 1;
    auto from = 0;
    if(row.is_heading()) {
      from = row.m_group->m_row;
      count += row.m_group->m_descendant_count;
    } else {
      from = m_indices[row.m_source.get_index()];
    }
    if(from != first) {
      for(auto i = 0; i != count; ++i) {
        move_row(from + i, first + i);
      }
    }
    first += count;
  }
}

void GroupedTableModel::sort_all() {
  m_unsorted.clear();
  sort_siblings(nullptr);
  for(auto& row : m_rows) {
    if(row.is_heading()) {
      sort_siblings(row.m_group);
    }
  }
}

void GroupedTableModel::sort_pending() {
  for(auto& group : m_unsorted) {
    if(!group || group->m_row != -1) {
      sort_siblings(group);
    }
  }
  m_unsorted.clear();
}

void GroupedTableModel::touch(const std::shared_ptr<Group>& group, int column) {
  auto& cell = group->m_aggregates[column];
  if(!cell.m_is_dirty) {
    cell.m_is_dirty = true;
    m_dirty.emplace_back(group, column);
  }
}

std::unique_ptr<TableAggregator> GroupedTableModel::make_aggregator(
    int column) const {
  if(column < std::ssize(m_aggregators) && m_aggregators[column]) {
    return m_aggregators[column]();
  }
  return std::make_unique<DefaultTableAggregator>();
}

GroupedTableModel::Group* GroupedTableModel::find_group(const AnyRef& key,
    const Group* parent, std::optional<std::size_t> hash) const {
  if(hash) {
    auto [i, end] = m_groups.equal_range(*hash);
    for(; i != end; ++i) {
      auto group = i->second;
      if(group->m_parent.get() == parent && equal_keys(group->m_key, key)) {
        return group;
      }
    }
  }
  return nullptr;
}

std::shared_ptr<GroupedTableModel::Group> GroupedTableModel::make_group(
    const AnyRef& key, QString name, const std::shared_ptr<Group>& parent,
    int depth, int heading_column) {
  auto group = std::make_shared<Group>(to_any(key), std::move(name), parent,
    m_next_group, depth, -1, 0, 0,
    std::vector<AggregateCell>(get_column_size()));
  ++m_next_group;
  for(auto i = 0; i != get_column_size(); ++i) {
    if(i != heading_column) {
      group->m_aggregates[i].m_accumulator = make_aggregator(i);
    }
  }
  return group;
}

void GroupedTableModel::initialize_aggregates(
    Group& group, std::span<const int> sources) {
  for(auto column = 0; column != get_column_size(); ++column) {
    auto& cell = group.m_aggregates[column];
    if(cell.m_accumulator) {
      for(auto source : sources) {
        cell.m_accumulator->add(m_source->at(source, column));
      }
      cell.m_value = cell.m_accumulator->get();
    }
  }
}

void GroupedTableModel::accumulate(
    const std::shared_ptr<Group>& group, int source) {
  for(auto column = 0; column != get_column_size(); ++column) {
    if(auto& accumulator = group->m_aggregates[column].m_accumulator) {
      accumulator->add(m_source->at(source, column));
      touch(group, column);
    }
  }
}

void GroupedTableModel::insert(int source, std::shared_ptr<Group> parent) {
  auto first = 0;
  auto end = get_row_size();
  auto shared_depth = 0;
  if(parent) {
    first = parent->m_row + 1;
    end = first + parent->m_descendant_count;
    shared_depth = parent->m_depth + 1;
  }
  for(auto depth = shared_depth; depth != std::ssize(m_columns); ++depth) {
    auto column = m_columns[depth];
    auto key = m_source->at(source, column);
    auto hash = group_hash(key, parent.get());
    auto match = find_group(key, parent.get(), hash);
    if(match) {
      first = match->m_row;
    } else {
      auto grouped = [=] (const auto& row) {
        return row.m_group && row.m_group->m_depth >= depth;
      };
      if(first == end || grouped(m_rows[end - 1])) {
        first = end;
      } else {
        auto position = std::partition_point(
          m_rows.begin() + first, m_rows.begin() + end, grouped);
        first = static_cast<int>(position - m_rows.begin());
      }
      auto group =
        make_group(key, to_text(key), parent, depth, m_columns.front());
      initialize_aggregates(*group, std::span(&source, 1));
      group->m_row = first;
      if(!m_order.empty()) {
        m_unsorted.insert(parent);
      }
      m_rows.emplace(m_rows.begin() + first, group, TableRowIndexTracker(-1));
      if(hash) {
        m_groups.emplace(*hash, group.get());
      }
      index_rows(first + 1, get_row_size());
      for(auto ancestor = parent; ancestor; ancestor = ancestor->m_parent) {
        ++ancestor->m_descendant_count;
      }
      m_transaction.push(AddOperation(first));
    }
    parent = m_rows[first].m_group;
    ++first;
    end = first + parent->m_descendant_count;
  }
  auto previous_row = m_indices[source];
  auto position = std::ranges::partition_point(m_rows.begin() + first,
    m_rows.begin() + end, [=, this] (const Row& row) {
      if(m_columns.empty()) {
        return true;
      } else if(previous_row >= first && previous_row < end) {
        return row.m_source.get_index() < source;
      }
      return row.m_group && (!m_order.empty() ||
        row.m_source.get_index() < source);
    });
  auto index = static_cast<int>(position - m_rows.begin());
  if(!m_order.empty()) {
    m_unsorted.insert(parent);
  }
  if(previous_row == -1) {
    m_rows.emplace(position, parent, TableRowIndexTracker(source));
    index_rows(index, get_row_size());
    m_transaction.push(AddOperation(index));
  } else {
    m_rows[previous_row].m_group = parent;
    if(previous_row < index) {
      --index;
    }
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
    if(group->m_leaf_count != 0) {
      accumulate(group, source);
    }
    ++group->m_leaf_count;
    m_transaction.push(UpdateOperation(
      group->m_row, m_columns.front(), std::move(previous), group->label()));
  }
}

void GroupedTableModel::remove_row(int row) {
  m_transaction.push(PreRemoveOperation(row));
  if(m_rows[row].is_heading()) {
    auto& group = *m_rows[row].m_group;
    if(auto hash = group_hash(group.m_key, group.m_parent.get())) {
      auto [i, end] = m_groups.equal_range(*hash);
      for(; i != end; ++i) {
        if(i->second == &group) {
          m_groups.erase(i);
          break;
        }
      }
    }
    group.m_row = -1;
  } else {
    m_indices[m_rows[row].m_source.get_index()] = -1;
  }
  m_rows.erase(m_rows.begin() + row);
  index_rows(row, get_row_size());
  m_transaction.push(RemoveOperation(row));
}

void GroupedTableModel::move_row(int source, int destination) {
  if(source != destination) {
    move_element(m_rows, source, destination);
    index_rows(
      std::min(source, destination), std::max(source, destination) + 1);
    m_transaction.push(MoveOperation(source, destination));
  }
}

void GroupedTableModel::remove_from_groups(int source,
    std::shared_ptr<Group> group, int shared_depth, const Changes& changes) {
  auto removed = 0;
  while(group) {
    auto row = group->m_row;
    group->m_descendant_count -= removed;
    if(group->m_depth < shared_depth) {
      group = group->m_parent;
      continue;
    }
    --group->m_descendant_count;
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
  index_rows(0, get_row_size());
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
    auto parent = std::shared_ptr<Group>();
    for(auto i = group; i; i = i->m_parent) {
      auto column = m_columns[i->m_depth];
      if(!changes.contains(column)) {
        continue;
      }
      auto value = m_source->at(source.get_index(), column);
      if(!equal_keys(i->m_key, value)) {
        depth = i->m_depth;
        parent = i->m_parent;
      }
    }
    for(auto i = group; i; i = i->m_parent) {
      if(i->m_depth < depth) {
        for(auto& [column, previous] : changes) {
          auto& cell = i->m_aggregates[column];
          auto current = m_source->at(source.get_index(), column);
          if(column == m_columns[i->m_depth]) {
            auto name = to_text(current);
            if(name != i->m_name) {
              auto previous_label = i->label();
              i->m_name = std::move(name);
              m_transaction.push(UpdateOperation(i->m_row, m_columns.front(),
                std::move(previous_label), i->label()));
              if(is_sorted(column) || is_sorted(m_columns.front())) {
                m_unsorted.insert(i->m_parent);
              }
            }
          }
          if(cell.m_accumulator) {
            cell.m_accumulator->update(previous, current);
            touch(i, column);
          }
        }
      }
    }
    if(depth != static_cast<int>(m_columns.size())) {
      insert(source.get_index(), std::move(parent));
      remove_from_groups(source.get_index(), std::move(group), depth, changes);
      row = m_indices[source.get_index()];
    }
    for(auto& [column, previous] : changes) {
      if(is_sorted(column)) {
        m_unsorted.insert(m_rows[row].m_group);
      }
      m_transaction.push(UpdateOperation(row, column, std::move(previous),
        to_any(m_source->at(source.get_index(), column))));
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
      if(!m_is_equal(std::as_const(cell.m_value), std::as_const(value))) {
        if(is_sorted(column)) {
          m_unsorted.insert(group->m_parent);
        }
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
        for(auto& column : m_order) {
          if(column.m_index >= get_column_size()) {
            throw std::invalid_argument("Invalid sort column.");
          }
        }
      }
      m_indices.insert(m_indices.begin() + add.m_index, -1);
      if(add.m_index + 1 != m_source->get_row_size()) {
        track(add);
      }
      flush_updates(&operation);
      insert(add.m_index, nullptr);
    },
    [&] (const PreRemoveOperation& removal) {
      flush_updates(nullptr);
      auto row = m_indices[removal.m_index];
      auto group = m_rows[row].m_group;
      remove_row(row);
      remove_from_groups(removal.m_index, std::move(group), 0, {});
    },
    [&] (const RemoveOperation& removal) {
      m_indices.erase(m_indices.begin() + removal.m_index);
      if(removal.m_index != std::ssize(m_indices)) {
        track(PreRemoveOperation(removal.m_index));
      }
    },
    [&] (const MoveOperation& move) {
      track(move);
      if(m_order.empty()) {
        move_leaf(move.m_destination);
      } else {
        m_unsorted.insert(m_rows[m_indices[move.m_destination]].m_group);
      }
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
      sort_pending();
    },
    [&] (const auto&) {
      if(m_column_count && std::holds_alternative<AddOperation>(operation) &&
          m_source->get_row_size() == 1) {
        auto count = m_source->get_column_size();
        m_has_matching_columns = count == *m_column_count;
        if(!m_has_matching_columns) {
          qWarning(
            "Expected %d source columns, received %d.", *m_column_count, count);
        }
      }
      if(!m_has_matching_columns) {
        return;
      }
      if(is_identity()) {
        m_transaction.push(operation);
      } else if(m_is_source_transaction) {
        apply(operation);
      } else {
        transact([&] {
          apply(operation);
          flush_updates(nullptr);
          refresh_aggregates();
          sort_pending();
        });
      }
    });
}
