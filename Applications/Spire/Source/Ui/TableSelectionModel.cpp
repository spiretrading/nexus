#include "Spire/Ui/TableSelectionModel.hpp"
#include <QScopedValueRollback>
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  int& row_of(int& row) {
    return row;
  }

  int& row_of(TableIndex& index) {
    return index.m_row;
  }

  template<typename T>
  class SelectableList : public FilteredListModel<T> {
    public:
      SelectableList(std::shared_ptr<ListModel<T>> source,
        std::function<bool (T)> is_excluded)
        : FilteredListModel<T>(source, [=] (const auto& list, int index) {
            return is_excluded(list.get(index));
          }),
          m_source(std::move(source)),
          m_is_excluded(std::move(is_excluded)),
          m_pruned_index(-1) {}

      QValidator::State set(int index, const T& value) override {
        if(m_is_excluded(value)) {
          return QValidator::Invalid;
        }
        return FilteredListModel<T>::set(index, value);
      }

      QValidator::State push(const T& value) override {
        if(m_is_excluded(value)) {
          return QValidator::Invalid;
        }
        return m_source->push(value);
      }

      QValidator::State insert(const T& value, int index) override {
        if(m_is_excluded(value)) {
          return QValidator::Invalid;
        } else if(index == this->get_size()) {
          return m_source->insert(value, m_source->get_size());
        }
        return m_source->insert(value, this->index_to_source(index));
      }

      QValidator::State move(int source, int destination) override {
        return m_source->move(
          this->index_to_source(source), this->index_to_source(destination));
      }

      QValidator::State remove(int index) override {
        auto is_excluded = m_is_excluded(this->get(index));
        auto state = m_source->remove(this->index_to_source(index));
        if(state == QValidator::Invalid && is_excluded) {
          exclude(index);
          return QValidator::Acceptable;
        }
        return state;
      }

      void exclude(int index) {
        auto guard =
          QScopedValueRollback(m_pruned_index, this->index_to_source(index));
        this->set_filter([=, this] (const auto& list, int index) {
          return (m_pruned_index != -1 && (index == m_pruned_index ||
            this->index_from_source(index) == -1)) ||
            m_is_excluded(list.get(index));
        });
      }

      void start_transaction() override {
        m_source->start_transaction();
      }

      void end_transaction() override {
        m_source->end_transaction();
      }

    private:
      std::shared_ptr<ListModel<T>> m_source;
      std::function<bool (T)> m_is_excluded;
      int m_pruned_index;
  };

  template<typename T>
  void update(ListModel<T>& selection, const TableModel::Operation& operation) {
    auto is_removing =
      std::holds_alternative<TableModel::PreRemoveOperation>(operation);
    auto to_index = [=] (int offset, int size) {
      if(is_removing) {
        return offset;
      }
      return size - 1 - offset;
    };
    auto prune = [&] (int index) {
      if(selection.remove(index) == QValidator::Invalid) {
        if(auto list = dynamic_cast<SelectableList<T>*>(&selection)) {
          list->exclude(index);
        }
      }
    };
    auto pending = std::vector<std::pair<int, T>>();
    for(auto offset = 0; offset < selection.get_size();) {
      auto size = selection.get_size();
      auto i = to_index(offset, size);
      auto value = selection.get(i);
      auto& row = row_of(value);
      auto tracker = TableRowIndexTracker(row);
      tracker.update(operation);
      if(tracker.get_index() == -1) {
        prune(i);
      } else if(tracker.get_index() != row) {
        row = tracker.get_index();
        if(selection.set(i, value) == QValidator::Invalid) {
          prune(i);
        } else if(selection.get_size() < size) {
          pending.emplace_back(offset, value);
        }
      }
      if(selection.get_size() == size) {
        ++offset;
      }
    }
    for(auto i = std::ssize(pending); i-- > 0;) {
      auto& [offset, value] = pending[i];
      selection.insert(value, to_index(offset, selection.get_size() + 1));
    }
  }
}

TableSelectionModel::TableSelectionModel(
  std::shared_ptr<ItemSelectionModel> item_selection,
  std::shared_ptr<RowSelectionModel> row_selection,
  std::shared_ptr<ColumnSelectionModel> column_selection)
  : m_item_selection(std::move(item_selection)),
    m_row_selection(std::move(row_selection)),
    m_column_selection(std::move(column_selection)),
    m_is_updating(false) {}

std::shared_ptr<const TableSelectionModel::ItemSelectionModel>
    TableSelectionModel::get_item_selection() const {
  return m_item_selection;
}

const std::shared_ptr<TableSelectionModel::ItemSelectionModel>&
    TableSelectionModel::get_item_selection() {
  return m_item_selection;
}

std::shared_ptr<const TableSelectionModel::RowSelectionModel>
    TableSelectionModel::get_row_selection() const {
  return m_row_selection;
}

const std::shared_ptr<TableSelectionModel::RowSelectionModel>&
    TableSelectionModel::get_row_selection() {
  return m_row_selection;
}

std::shared_ptr<const TableSelectionModel::ColumnSelectionModel>
    TableSelectionModel::get_column_selection() const {
  return m_column_selection;
}

const std::shared_ptr<TableSelectionModel::ColumnSelectionModel>&
    TableSelectionModel::get_column_selection() {
  return m_column_selection;
}

bool TableSelectionModel::is_active() const {
  return !m_source_tracking || m_source_tracking->connected();
}

connection TableSelectionModel::track(
    const std::shared_ptr<TableModel>& table) {
  if(m_table.lock() == table) {
    return m_table_connection;
  }
  if(m_filter_signal) {
    m_filter_signal->disconnect_all_slots();
  } else {
    m_filter_signal = std::make_shared<FilterSignal>();
    auto is_excluded = [signal = m_filter_signal] (auto value) {
      auto excluded = false;
      (*signal)(row_of(value), excluded);
      return excluded;
    };
    m_row_selection = std::make_shared<SelectableList<int>>(
      m_row_selection, is_excluded);
    m_item_selection = std::make_shared<SelectableList<TableIndex>>(
      m_item_selection, is_excluded);
  }
  m_table = table;
  m_table_connection = table->connect_operation_signal(
    [=, this] (const auto& operation) {
      visit(operation,
        [&] (const TableModel::RemoveOperation& remove) {
          update_rows(TableModel::PreRemoveOperation(remove.m_index));
        },
        [&] (const TableRowChange auto&) {
          update_rows(operation);
        });
    });
  return m_table_connection;
}

void TableSelectionModel::update_rows(const TableModel::Operation& operation) {
  if(!m_source_tracking) {
    auto guard = QScopedValueRollback(m_is_updating, true);
    transact([&] {
      update(*m_row_selection, operation);
      update(*m_item_selection, operation);
    });
  }
}
