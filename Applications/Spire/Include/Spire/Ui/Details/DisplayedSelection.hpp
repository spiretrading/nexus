#ifndef SPIRE_DISPLAYED_SELECTION_HPP
#define SPIRE_DISPLAYED_SELECTION_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  template<typename T>
  class TableView::DisplayedSelection : public ArrayListModel<T> {
    public:
      DisplayedSelection(std::shared_ptr<ListModel<T>> source,
        std::shared_ptr<FilteredTableModel> filtered,
        std::shared_ptr<SortedTableModel> sorted, auto is_updating,
        boost::signals2::connection tracking);

      void track(auto refresh);
      void refresh();
      QValidator::State set(int index, const T& value) override;
      QValidator::State push(const T& value) override;
      QValidator::State insert(const T& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      std::shared_ptr<ListModel<T>> m_source;
      std::shared_ptr<FilteredTableModel> m_filtered;
      std::shared_ptr<SortedTableModel> m_sorted;
      std::vector<int> m_indices;
      boost::signals2::connection m_tracking_connection;
      boost::signals2::scoped_connection m_connection;
      boost::signals2::scoped_connection m_source_connection;
      boost::signals2::scoped_connection m_view_connection;

      boost::optional<T> from_source(T value) const;
      void append(int index);
      boost::optional<T> to_source(T value) const;
  };

  template<typename T>
  TableView::DisplayedSelection<T>::DisplayedSelection(
      std::shared_ptr<ListModel<T>> source,
      std::shared_ptr<FilteredTableModel> filtered,
      std::shared_ptr<SortedTableModel> sorted, auto is_updating,
      boost::signals2::connection tracking)
      : m_source(std::move(source)),
        m_filtered(std::move(filtered)),
        m_sorted(std::move(sorted)),
        m_tracking_connection(std::move(tracking)) {
    m_connection = m_source->connect_operation_signal(
      [=, this] (const auto& operation) {
        if(is_updating()) {
          return;
        }
        if(auto add =
            std::get_if<typename ListModel<T>::AddOperation>(&operation)) {
          if(add->m_index == m_source->get_size() - 1) {
            append(add->m_index);
            return;
          }
        } else if(auto removal = std::get_if<
            typename ListModel<T>::RemoveOperation>(&operation)) {
          if(!m_indices.empty() && removal->m_index == m_indices.back()) {
            m_indices.pop_back();
            ArrayListModel<T>::remove(this->get_size() - 1);
            return;
          }
        }
        if(!std::holds_alternative<
            typename ListModel<T>::PreRemoveOperation>(operation) &&
            !std::holds_alternative<
              typename ListModel<T>::StartTransaction>(operation) &&
            !std::holds_alternative<
              typename ListModel<T>::EndTransaction>(operation)) {
          refresh();
        }
      });
    refresh();
  }

  template<typename T>
  void TableView::DisplayedSelection<T>::track(auto refresh) {
    auto& source = *m_filtered->get_source();
    m_source_connection = source.connect_operation_signal(
      [=, this] (const auto& operation) {
        if(auto add = std::get_if<TableModel::AddOperation>(&operation)) {
          if(m_filtered->index_from_source(add->m_index) != -1) {
            return;
          }
        }
        if(is_row_change(operation)) {
          refresh(*this);
        }
      });
    m_view_connection = m_sorted->connect_operation_signal(
      [=, this, has_pending_move = false] (const auto& operation) mutable {
        if(std::holds_alternative<TableModel::MoveOperation>(operation)) {
          has_pending_move = true;
        } else if(is_row_change(operation) || (has_pending_move &&
            std::holds_alternative<TableModel::EndTransaction>(operation))) {
          has_pending_move = false;
          refresh(*this);
        }
      });
  }
}

#endif
