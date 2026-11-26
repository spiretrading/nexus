#include "Spire/Ui/GroupedTableModel.hpp"
#include <algorithm>
#include <optional>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/scope/scope_exit.hpp>
#include "Spire/Spire/TableRowIndexTracker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Details/DefaultTableAggregation.hpp"

using namespace boost;
using namespace boost::scope;
using namespace Spire;

namespace {
  using boost::multiprecision::cpp_bin_float_quad;

  template<typename... T>
  AnyRef make_numeric_key(AnyRef value) {
    auto key = std::optional<cpp_bin_float_quad>();
    auto convert = [&]<typename N> {
      if(auto number = any_cast<N>(&std::as_const(value))) {
        key.emplace(*number);
      } else if(auto number = any_cast<optional<N>>(&std::as_const(value))) {
        if(*number) {
          key.emplace(**number);
        }
      } else {
        return false;
      }
      return true;
    };
    if((convert.template operator()<T>() || ...)) {
      if(key) {
        return AnyRef(*key, AnyRef::by_value);
      }
      return AnyRef();
    }
    return value;
  }

  AnyRef make_key(AnyRef value) {
    if(value.get_type() == typeid(QString) ||
        value.get_type() == typeid(std::string)) {
      return value;
    }
    return make_numeric_key<float, double, long double>(std::move(value));
  }

  bool equal_keys(const std::any& left, const AnyRef& right) {
    if(left.type() != right.get_type()) {
      return false;
    } else if(auto text = std::any_cast<QString>(&left)) {
      return *text == any_cast<QString>(right);
    } else if(!left.has_value()) {
      return true;
    } else if(auto a = std::any_cast<cpp_bin_float_quad>(&left)) {
      auto& b = any_cast<cpp_bin_float_quad>(right);
      return *a == b || (isnan(*a) && isnan(b));
    }
    return Spire::is_equal(left, right);
  }

  std::optional<std::size_t> group_hash(const AnyRef& key, const void* parent) {
    auto hash = [&] () -> std::optional<std::size_t> {
      if(!key.has_value()) {
        return 0;
      } else if(auto number = any_cast<cpp_bin_float_quad>(&key)) {
        if(*number == 0 || isnan(*number)) {
          return 0;
        }
        return hash_value(*number);
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
    } else if(auto a = any_cast<cpp_bin_float_quad>(&left)) {
      auto b = any_cast<cpp_bin_float_quad>(&right);
      return b && !isnan(*a) && (isnan(*b) || *a < *b);
    }
    return compare(left, right);
  }

  struct AggregateCell {
    std::any m_value;
    std::unique_ptr<TableAggregation> m_accumulator;
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

GroupedTableModel::GroupedTableModel(
  std::shared_ptr<TableModel> source, std::vector<int> columns)
  : GroupedTableModel(std::move(source), std::move(columns), {}) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
  std::vector<int> columns, std::vector<TableAggregationFactory> aggregations)
  : GroupedTableModel(std::move(source), std::move(columns),
      std::move(aggregations), Comparator()) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
  std::vector<int> columns, std::vector<TableAggregationFactory> aggregations,
  ValueComparator comparator)
  : GroupedTableModel(std::move(source), std::move(columns),
      std::move(aggregations), [comparator = std::move(comparator)] (
          const AnyRef& left, int, const AnyRef& right, int, int) {
        return comparator(left, right);
      }) {}

GroupedTableModel::GroupedTableModel(std::shared_ptr<TableModel> source,
    std::vector<int> columns, std::vector<TableAggregationFactory> aggregations,
    Comparator comparator)
    : m_source(std::move(source)),
      m_has_matching_columns(true),
      m_columns(std::move(columns)),
      m_aggregations(std::move(aggregations)),
      m_uses_default_comparator(!comparator),
      m_comparator(std::move(comparator)),
      m_next_group(0),
      m_is_source_transaction(false) {
  if(!m_comparator) {
    m_comparator = &compare_siblings;
  }
  validate_columns(m_columns);
  if(m_columns.empty()) {
    m_transaction.set_size(m_source->get_row_size());
  } else {
    m_indices.resize(m_source->get_row_size(), -1);
    for(auto i = 0; i != m_source->get_row_size(); ++i) {
      insert(i, nullptr);
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
    if(is_identity()) {
      materialize_rows();
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
  auto key = [] (const SortedTableModel::ColumnOrder& column) {
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
  auto& entry = m_rows.at(row);
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
  return m_rows[row].m_source.get_index();
}

int GroupedTableModel::index_from_source(int row) const {
  if(is_identity()) {
    return index_to_source(row);
  } else if(row < 0 || row >= std::ssize(m_indices)) {
    return -1;
  }
  return m_indices[row];
}

int GroupedTableModel::get_row_size() const {
  if(is_identity()) {
    return get_source_row_size();
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
  if(has_width && std::ssize(m_aggregations) > width) {
    throw std::invalid_argument("Grouping exceeds the source column count.");
  }
}

bool GroupedTableModel::is_identity() const {
  return m_columns.empty() && m_order.empty();
}

void GroupedTableModel::materialize_rows() {
  for(auto i = 0; i != get_source_row_size(); ++i) {
    m_rows.emplace_back(nullptr, TableRowIndexTracker(i));
    m_indices.push_back(i);
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
  auto& cell = group.m_aggregates[column];
  if(Details::is_default_table_aggregation(cell.m_accumulator.get())) {
    if(auto count = std::any_cast<QString>(&cell.m_value)) {
      return static_cast<std::int64_t>(
        QStringView(*count).mid(1, count->size() - 2).toLongLong());
    } else if(m_uses_default_comparator) {
      if(auto sum = std::any_cast<std::string>(&cell.m_value)) {
        return cpp_bin_float_quad(*sum);
      }
      return make_numeric_key<std::int64_t, std::uint64_t>(cell.m_value);
    }
  }
  return cell.m_value;
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

std::unique_ptr<TableAggregation> GroupedTableModel::make_aggregation(
    const Group& group, int column) const {
  auto accumulator = [&] {
    if(column < std::ssize(m_aggregations) && m_aggregations[column]) {
      return m_aggregations[column]();
    }
    return make_default_table_aggregation();
  }();
  if(!Details::is_default_table_aggregation(accumulator.get())) {
    return accumulator;
  }
  Details::set_table_value_visitor(*accumulator,
    [&, column, this] (const Details::TableValueVisitor& visit) {
      auto end = group.m_row + group.m_descendant_count;
      for(auto row = group.m_row + 1; row <= end; ++row) {
        if(!m_rows[row].is_heading()) {
          visit(m_source->at(m_rows[row].m_source.get_index(), column));
        }
      }
    });
  return accumulator;
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
    auto value = m_source->at(source, column);
    auto key = make_key(value);
    auto hash = group_hash(key, parent.get());
    auto match = static_cast<Group*>(nullptr);
    if(hash) {
      auto [i, last] = m_groups.equal_range(*hash);
      for(; i != last; ++i) {
        auto group = i->second;
        if(group->m_parent == parent && equal_keys(group->m_key, key)) {
          match = group;
          break;
        }
      }
    }
    if(match) {
      first = match->m_row;
    } else {
      auto grouped = [=] (const Row& row) {
        return row.m_group && row.m_group->m_depth >= depth;
      };
      if(first == end || grouped(m_rows[end - 1])) {
        first = end;
      } else {
        auto position = std::partition_point(
          m_rows.begin() + first, m_rows.begin() + end, grouped);
        first = static_cast<int>(position - m_rows.begin());
      }
      auto group = std::make_shared<Group>(to_any(key), to_text(value), parent,
        m_next_group, depth, first, 0, 0,
        std::vector<AggregateCell>(get_column_size()));
      ++m_next_group;
      if(!m_order.empty()) {
        m_unsorted.insert(parent);
      }
      for(auto i = 0; i != get_column_size(); ++i) {
        if(i != m_columns.front()) {
          group->m_aggregates[i].m_accumulator = make_aggregation(*group, i);
        }
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
      if(!equal_keys(i->m_key, make_key(value))) {
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
      if(!Details::is_default_table_aggregation(cell.m_accumulator.get()) ||
          !is_equal(cell.m_value, value)) {
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
