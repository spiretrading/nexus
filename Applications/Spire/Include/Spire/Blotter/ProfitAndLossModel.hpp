#ifndef SPIRE_PROFITANDLOSSMODEL_HPP
#define SPIRE_PROFITANDLOSSMODEL_HPP
#include <unordered_map>
#include <Beam/Pointers/DelayPtr.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/signals2/signal.hpp>
#include <QTimer>
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class ProfitAndLossModel
      \brief Models the profit and loss of a blotter.
   */
  class ProfitAndLossModel : public QObject {
    public:

      //! Signals an update to the total profit and loss.
      /*!
        \param update Stores the update to the total profit and loss.  Only
               the currency entries are populated.
      */
      typedef boost::signals2::signal<
        void (const SpirePortfolioMonitor::UpdateEntry& update)>
        ProfitAndLossUpdateSignal;

      //! Signals that a ProfitAndLossEntryModel was added.
      /*!
        \param model The ProfitAndLossEntryModel that was added.
      */
      typedef boost::signals2::signal<void (ProfitAndLossEntryModel& model)>
        ProfitAndLossEntryModelAddedSignal;

      //! Signals that a ProfitAndLossEntryModel was removed.
      /*!
        \param model The ProfitAndLossEntryModel that was removed.
      */
      typedef boost::signals2::signal<void (ProfitAndLossEntryModel& model)>
        ProfitAndLossEntryModelRemovedSignal;

      //! Defines the factory used to build a PortfolioMonitor.
      typedef std::function<std::unique_ptr<SpirePortfolioMonitor> ()>
        PortfolioMonitorFactory;

      //! Constructs a ProfitAndLossModel.
      /*!
        \param currencyDatabase Stores the database of all currencies.
        \param exchangeRates The list of exchange rates.
      */
      ProfitAndLossModel(
        Beam::RefType<const Nexus::CurrencyDatabase> currencyDatabase,
        Beam::RefType<const Nexus::ExchangeRateTable> exchangeRates);

      virtual ~ProfitAndLossModel();

      //! Sets the PortfolioMonitor whose profit and loss is to be modeled.
      /*!
        \param portfolioMonitor The PortfolioMonitor whose profit and loss is to
               be modeled.
      */
      void SetPortfolioMonitor(Beam::RefType<SpirePortfolioMonitor>
        portfolioMonitor);

      //! Set the currency that profit and loss updates are expressed in.
      void SetCurrency(Nexus::CurrencyId currency);

      //! Connects a slot to the ProfitAndLossUpdated signal.
      boost::signals2::connection ConnectProfitAndLossUpdateSignal(
        const ProfitAndLossUpdateSignal::slot_type& slot) const;

      //! Connects a slot to the ProfitAndLossEntryModelAdded signal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectProfitAndLossEntryModelAddedSignal(
        const ProfitAndLossEntryModelAddedSignal::slot_type& slot) const;

      //! Connects a slot to the ProfitAndLossEntryModelRemoved signal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectProfitAndLossEntryModelRemovedSignal(
        const ProfitAndLossEntryModelRemovedSignal::slot_type& slot) const;

    private:
      const Nexus::CurrencyDatabase* m_currencyDatabase;
      const Nexus::ExchangeRateTable* m_exchangeRates;
      SpirePortfolioMonitor* m_portfolioMonitor;
      Nexus::CurrencyId m_currency;
      QTimer m_updateTimer;
      std::unordered_map<Nexus::CurrencyId, SpirePortfolioMonitor::UpdateEntry>
        m_currencyToPortfolio;
      std::unordered_map<Nexus::CurrencyId,
        std::unique_ptr<ProfitAndLossEntryModel>> m_currencyToModel;
      SpirePortfolioMonitor::UpdateEntry m_update;
      mutable ProfitAndLossUpdateSignal m_profitAndLossUpdateSignal;
      mutable ProfitAndLossEntryModelAddedSignal
        m_profitAndLossEntryModelAddedSignal;
      mutable ProfitAndLossEntryModelRemovedSignal
        m_profitAndLossEntryModelRemovedSignal;
      Beam::DelayPtr<Beam::TaskQueue> m_slotHandler;

      void OnPortfolioUpdate(const SpirePortfolioMonitor::UpdateEntry& update);
      void OnUpdateTimer();
  };
}

#endif
