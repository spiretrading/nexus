#ifndef SPIRE_BLOTTER_COLLECTION_HPP
#define SPIRE_BLOTTER_COLLECTION_HPP
#include <unordered_map>
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Owns blotters and coordinates their links. */
  class BlotterCollection : public BlottersModel {
    public:

      /**
       * Constructs an empty collection.
       * @param client Supplies market prices for valuation.
       */
      explicit BlotterCollection(Nexus::MarketDataClient client);

      ~BlotterCollection() override;

      /**
       * Adds a blotter's local models.
       * @param source The local models, including the account portfolio for a
       *        consolidated blotter.
       * @return The managed blotter, with its linked tasks and orders.
       */
      Type add(Type source);

      /** Returns a blotter's direct links. */
      std::shared_ptr<const BlottersModel>
        get_links(const BlotterModel& blotter) const;

      /** Links a receiving blotter to a source blotter. */
      QValidator::State link(
        const BlotterModel& receiver, const BlotterModel& source);

      /** Removes a receiving blotter's link to a source blotter. */
      QValidator::State unlink(
        const BlotterModel& receiver, const BlotterModel& source);

      int get_size() const override;
      const Type& get(int index) const override;
      QValidator::State remove(int index) override;
      using BlottersModel::remove;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      struct Entry;
      Nexus::MarketDataClient m_client;
      ArrayListModel<Type> m_blotters;
      std::unordered_map<const BlotterModel*, std::unique_ptr<Entry>> m_entries;

      Entry* find(const BlotterModel& blotter) const;
      bool is_valid(
        const BlotterModel& receiver, const BlotterModel& source) const;
      void refresh();
  };
}

#endif
