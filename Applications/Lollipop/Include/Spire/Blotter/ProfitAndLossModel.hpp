#ifndef SPIRE_PROFIT_AND_LOSS_MODEL_HPP
#define SPIRE_PROFIT_AND_LOSS_MODEL_HPP
#include <optional>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Nexus/MarketDataService/MarketDataClient.hpp"
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Blotter/Blotter.hpp"

namespace Spire {

  /** Models the profit and loss of a blotter. */
  class ProfitAndLossModel : public QObject {
    public:

      /** The type of PortfolioController managing updates. */
      using PortfolioController = Nexus::PortfolioController<
        Nexus::Portfolio<Nexus::TrueAverageBookkeeper>,
        Nexus::MarketDataClient>;

      /**
       * Signals an update to the total profit and loss.
       * @param update Stores the update to the total profit and loss. Only the
       *        currency entries are populated.
       */
      using ProfitAndLossUpdateSignal = boost::signals2::signal<
        void (const Nexus::PortfolioUpdateEntry& update)>;

      /**
       * Signals that a ProfitAndLossEntryModel was added.
       * @param model The ProfitAndLossEntryModel that was added.
       */
      using ProfitAndLossEntryModelAddedSignal =
        boost::signals2::signal<void (ProfitAndLossEntryModel& model)>;

      /**
       * Signals that a ProfitAndLossEntryModel was removed.
       * @param model The ProfitAndLossEntryModel that was removed.
       */
      using ProfitAndLossEntryModelRemovedSignal =
        boost::signals2::signal<void (ProfitAndLossEntryModel& model)>;

      /** Defines the factory used to build a PortfolioController. */
      using PortfolioControllerFactory =
        std::function<std::unique_ptr<PortfolioController> ()>;

      /**
       * Constructs a ProfitAndLossModel.
       * @param currencyDatabase Stores the database of all currencies.
       * @param exchangeRates The list of exchange rates.
       */
      ProfitAndLossModel(
        Beam::Ref<const Nexus::CurrencyDatabase> currencyDatabase,
        Beam::Ref<const Nexus::ExchangeRateTable> exchangeRates,
        bool showUnrealized);

      ~ProfitAndLossModel() override;

      /**
       * Sets the PortfolioController whose profit and loss is to be modeled.
       * @param portfolioController The PortfolioController whose profit and
       *        loss is to be modeled.
       */
      void SetPortfolioController(
        Beam::Ref<PortfolioController> portfolioController);

      /** Set the currency that profit and loss updates are expressed in. */
      void SetCurrency(Nexus::Asset currency);

      /** Connects a slot to the ProfitAndLossUpdated signal. */
      boost::signals2::connection ConnectProfitAndLossUpdateSignal(
        const ProfitAndLossUpdateSignal::slot_type& slot) const;

      /**
       * Connects a slot to the ProfitAndLossEntryModelAdded signal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectProfitAndLossEntryModelAddedSignal(
        const ProfitAndLossEntryModelAddedSignal::slot_type& slot) const;

      /**
       * Connects a slot to the ProfitAndLossEntryModelRemoved signal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectProfitAndLossEntryModelRemovedSignal(
        const ProfitAndLossEntryModelRemovedSignal::slot_type& slot) const;

    private:
      const Nexus::CurrencyDatabase* m_currencyDatabase;
      const Nexus::ExchangeRateTable* m_exchangeRates;
      bool m_showUnrealized;
      PortfolioController* m_portfolioController;
      Nexus::Asset m_currency;
      std::unordered_map<Nexus::Asset, Nexus::PortfolioUpdateEntry>
        m_currencyToPortfolio;
      std::unordered_map<Nexus::Asset,
        std::unique_ptr<ProfitAndLossEntryModel>> m_currencyToModel;
      Nexus::PortfolioUpdateEntry m_update;
      mutable ProfitAndLossUpdateSignal m_profitAndLossUpdateSignal;
      mutable ProfitAndLossEntryModelAddedSignal
        m_profitAndLossEntryModelAddedSignal;
      mutable ProfitAndLossEntryModelRemovedSignal
        m_profitAndLossEntryModelRemovedSignal;
      std::optional<EventHandler> m_eventHandler;

      void OnPortfolioUpdate(const Nexus::PortfolioUpdateEntry& update);
  };
}

#endif
