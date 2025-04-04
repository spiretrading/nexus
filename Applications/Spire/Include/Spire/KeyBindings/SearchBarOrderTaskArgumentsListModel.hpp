#ifndef SPIRE_SEARCH_BAR_ORDER_TASK_ARGUMENTS_LIST_MODEL_HPP
#define SPIRE_SEARCH_BAR_ORDER_TASK_ARGUMENTS_LIST_MODEL_HPP
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/OrderTaskArguments.hpp"
#include "Spire/Spire/FilteredListModel.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {

  /** Filters an OrderTaskArgumentsListModel based on a keyword search. */
  class SearchBarOrderTaskArgumentsListModel :
      public OrderTaskArgumentsListModel {
    public:

      /**
       * Constructs a SearchBarOrderTaskArgumentsListModel that does not
       * initially filter out any elements.
       * @param source The list to perform the keyword filtering on.
       * @param keywords The keywords used to filter out elements.
       * @param countries The database of countries used to filter by country
       *        name or short form.
       * @param markets The database of markets used to filter by market name
       *        or short form.
       * @param destinations The database of destinations used to filter by
       *        destination name or short form.
       */
      SearchBarOrderTaskArgumentsListModel(
        std::shared_ptr<OrderTaskArgumentsListModel> source,
        std::shared_ptr<TextModel> keywords, Nexus::CountryDatabase countries,
        Nexus::MarketDatabase markets, Nexus::DestinationDatabase destinations);

      int get_size() const override;

      const OrderTaskArguments& get(int index) const override;

      QValidator::State
        set(int index, const OrderTaskArguments& value) override;

      QValidator::State push(const OrderTaskArguments& value) override;

      QValidator::State
        insert(const OrderTaskArguments& value, int index) override;

      QValidator::State move(int source, int destination) override;

      QValidator::State remove(int index) override;

      boost::signals2::connection connect_operation_signal(
        const typename OperationSignal::slot_type& slot) const override;

    protected:
      void transact(const std::function<void ()>& transaction) override;

    private:
      Nexus::CountryDatabase m_countries;
      Nexus::MarketDatabase m_markets;
      Nexus::DestinationDatabase m_destinations;
      FilteredListModel<OrderTaskArguments> m_filtered_list;
      std::shared_ptr<TextModel> m_keywords;
      boost::signals2::scoped_connection m_connection;

      void on_keywords(const QString& keywords);
  };
}

#endif
