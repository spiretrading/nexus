#ifndef SPIRE_DISPLAYED_COLUMN_SELECTION_HPP
#define SPIRE_DISPLAYED_COLUMN_SELECTION_HPP
#include "Spire/Ui/TableView.hpp"

namespace Spire {
  class TableView::DisplayedColumnSelection : public ListModel<int> {
    public:
      DisplayedColumnSelection(std::shared_ptr<TableSelectionModel> selection,
        boost::signals2::connection tracking);

      int get_size() const override;
      const int& get(int index) const override;
      QValidator::State set(int index, const int& value) override;
      QValidator::State push(const int& value) override;
      QValidator::State insert(const int& value, int index) override;
      QValidator::State move(int source, int destination) override;
      QValidator::State remove(int index) override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      std::shared_ptr<TableSelectionModel> m_selection;
      int m_inactive_depth;
      boost::signals2::connection m_tracking_connection;
  };
}

#endif
