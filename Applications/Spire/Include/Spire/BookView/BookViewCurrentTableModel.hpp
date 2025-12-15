#ifndef SPIRE_BOOK_VIEW_CURRENT_TABLE_MODEL_HPP
#define SPIRE_BOOK_VIEW_CURRENT_TABLE_MODEL_HPP
#include <memory>
#include "Spire/Spire/TableCurrentIndexModel.hpp"
#include "Spire/Ui/TableCurrentController.hpp"

namespace Spire {

  /**
   * Implements a CurrentModel for a TableModel that is restricted to the
   * selection of UserOrders.
   */
  class BookViewCurrentTableModel :
      public TableCurrentController::CurrentModel {
    public:

      /**
       * Constructs a BookViewCurrentTableModel.
       * @param table The TableModel to index.
       */
      explicit BookViewCurrentTableModel(std::shared_ptr<TableModel> table);

      const Type& get() const override;
      QValidator::State test(const Type& value) const override;
      QValidator::State set(const Type& value) override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      std::shared_ptr<TableModel> m_table;
      TableCurrentIndexModel m_current;
  };
}

#endif
