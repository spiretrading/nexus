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
       * @param countries The database of countries used to filter by country
       *        name or short form.
       * @param markets The database of markets used to filter by market name
       *        or short form.
       * @param destinations The database of destinations used to filter by
       *        destination name or short form.
       */
      KeywordFilteredTableModel(std::shared_ptr<TableModel> source,
        std::shared_ptr<TextModel> keywords,
        const Nexus::CountryDatabase& countries,
        const Nexus::MarketDatabase& markets,
        const Nexus::DestinationDatabase& destinations);

      int get_row_size() const override;

      int get_column_size() const override;

      AnyRef at(int row, int column) const override;

      QValidator::State
        set(int row, int column, const std::any& value) override;

      QValidator::State remove(int row) override;

      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;

    private:
      Nexus::CountryDatabase m_countries;
      Nexus::MarketDatabase m_markets;
      Nexus::DestinationDatabase m_destinations;
      FilteredTableModel m_filtered_model;
      std::shared_ptr<TextModel> m_keywords;
      boost::signals2::scoped_connection m_connection;

      void on_keywords(const QString& keywords);
  };
}

#endif
