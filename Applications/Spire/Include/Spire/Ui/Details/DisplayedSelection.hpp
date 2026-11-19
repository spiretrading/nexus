#ifndef SPIRE_DISPLAYED_SELECTION_HPP
#define SPIRE_DISPLAYED_SELECTION_HPP
#include <utility>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  template<typename T>
  class TableView::DisplayedSelection : public ArrayListModel<T> {
    public:
      DisplayedSelection(std::shared_ptr<ListModel<T>> source,
        std::shared_ptr<FilteredTableModel> filtered,
        std::shared_ptr<SortedTableModel> sorted,
        std::shared_ptr<const TranslatedListModel<int>> columns,
        auto is_updating,
        boost::signals2::connection tracking);

      void track(auto refresh);
      void refresh();
      void refresh(const AnyListModel::MoveOperation& column_move);
      const T& get(int index) const override;
      QValidator::State set(int index, const T& value) override;
      QValidator::State push(const T& value) override;
      QValidator::State insert(const T& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      std::shared_ptr<ListModel<T>> m_source;
      std::shared_ptr<FilteredTableModel> m_filtered;
      std::shared_ptr<SortedTableModel> m_sorted;
      std::shared_ptr<const TranslatedListModel<int>> m_columns;
      std::vector<int> m_indices;
      mutable std::vector<T> m_transaction_values;
      bool m_is_refresh_pending;
      bool m_is_pruning;
      int m_inactive_depth;
      boost::signals2::connection m_tracking_connection;
      boost::signals2::scoped_connection m_connection;
      boost::signals2::scoped_connection m_source_connection;
      boost::signals2::scoped_connection m_view_connection;
      boost::signals2::scoped_connection m_column_connection;

      bool is_tail_append(int row) const;
      boost::optional<T> from_source(T value) const;
      void append(int index);
      boost::optional<T> to_source(T value) const;
  };

  template<typename T>
  TableView::DisplayedSelection<T>::DisplayedSelection(
      std::shared_ptr<ListModel<T>> source,
      std::shared_ptr<FilteredTableModel> filtered,
      std::shared_ptr<SortedTableModel> sorted,
      std::shared_ptr<const TranslatedListModel<int>> columns, auto is_updating,
      boost::signals2::connection tracking)
      : m_source(std::move(source)),
        m_filtered(std::move(filtered)),
        m_sorted(std::move(sorted)),
        m_columns(std::move(columns)),
        m_is_refresh_pending(false),
        m_is_pruning(false),
        m_inactive_depth(0),
        m_tracking_connection(std::move(tracking)) {
    m_connection = m_source->connect_operation_signal(
      [=, this, depth = 0] (const auto& operation) mutable {
        auto is_updating_selection = m_is_pruning ||
          (m_filtered && is_updating());
        visit(operation,
          [&] (const AnyListModel::StartTransaction&) {
            ++depth;
            ArrayListModel<T>::start_transaction();
          },
          [&] (const AnyListModel::EndTransaction&) {
            if(depth != 0) {
              --depth;
              ArrayListModel<T>::end_transaction();
            }
          },
          [&] (const AnyListModel::AddOperation& add) {
            if(is_updating_selection) {
              return;
            }
            if(add.m_index == m_source->get_size() - 1) {
              append(add.m_index);
            } else {
              refresh();
            }
          },
          [&] (const AnyListModel::RemoveOperation& removal) {
            if(is_updating_selection) {
              return;
            }
            if(!m_indices.empty() && removal.m_index == m_indices.back()) {
              m_indices.pop_back();
              ArrayListModel<T>::remove(this->get_size() - 1);
            } else {
              refresh();
            }
          },
          [&] <typename O> (const O&) requires
              (std::derived_from<O, AnyListModel::MoveOperation> ||
                std::derived_from<O, AnyListModel::UpdateOperation>) {
            if(!is_updating_selection) {
              refresh();
            }
          });
      });
    refresh();
  }

  template<typename T>
  void TableView::DisplayedSelection<T>::track(auto refresh) {
    if(m_columns) {
      m_column_connection = m_columns->connect_operation_signal(
        [=, this] (const auto& operation) {
          visit(operation, [&] (const AnyListModel::MoveOperation& move) {
            refresh(*this, move);
          });
        });
    }
    if(!m_filtered) {
      return;
    }
    auto& source = *m_filtered->get_source();
    m_source_connection = source.connect_operation_signal(
      [=, this, is_transacting = false] (
          const TableModel::Operation& operation) mutable {
        visit(operation,
          [&] (const TableModel::StartTransaction&) {
            is_transacting = true;
          },
          [&] (const TableModel::EndTransaction&) {
            is_transacting = false;
            if(m_is_refresh_pending) {
              refresh(*this);
            }
          },
          [&] (const TableModel::AddOperation& add) {
            if(m_is_refresh_pending ||
                (add.m_index != m_filtered->get_source()->get_row_size() - 1 &&
                  m_filtered->index_from_source(add.m_index) == -1)) {
              refresh(*this);
            }
          },
          [&] (const TableModel::UpdateOperation& update) {
            if(m_is_refresh_pending &&
                m_filtered->index_from_source(update.m_row) == -1) {
              refresh(*this);
            }
          },
          [&] (const TableRowChange auto&) {
            if(is_transacting && m_source->get_size() == this->get_size()) {
              m_is_refresh_pending = true;
            } else {
              refresh(*this);
            }
          });
      });
    m_view_connection = m_sorted->connect_operation_signal(
      [=, this, is_transacting = false] (
          const TableModel::Operation& operation) mutable {
        visit(operation,
          [&] (const TableModel::StartTransaction&) {
            is_transacting = true;
          },
          [&] (const TableModel::EndTransaction&) {
            is_transacting = false;
            if(m_is_refresh_pending) {
              refresh(*this);
            }
          },
          [&] <TableRowChange O> (const O& change) {
            if constexpr(std::same_as<O, TableModel::AddOperation>) {
              if(is_tail_append(change.m_index) &&
                  m_source->get_size() == this->get_size()) {
                return;
              }
            }
            if(is_transacting) {
              m_is_refresh_pending = true;
            } else {
              refresh(*this);
            }
          });
      });
  }
}

#endif
