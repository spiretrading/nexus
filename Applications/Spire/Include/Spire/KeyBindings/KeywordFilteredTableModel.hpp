#ifndef SPIRE_KEYWORD_FILTERED_TABLE_MODEL_HPP
#define SPIRE_KEYWORD_FILTERED_TABLE_MODEL_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/FilteredTableModel.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {
  class KeywordFilteredTableModel : public TableModel {
    public:

      /**
       * Constructs a KeywordFilteredTableModel that does not initially filter
       * out any rows.
       * @param source The table to perform the keyword filtering on.
       * @param keywords The keywords used to filter out rows.
       */
      KeywordFilteredTableModel(std::shared_ptr<TableModel> source,
        std::shared_ptr<TextModel> keywords);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State
        set(int row, int column, const std::any& value) override;

      QValidator::State remove(int row) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      FilteredTableModel m_filtered_model;
      std::shared_ptr<TextModel> m_keywords;
      boost::signals2::scoped_connection m_connection;

      void on_keywords(const QString& keywords);
  };
}

#endif
