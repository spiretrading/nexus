#ifndef SPIRE_CONTROLLER_PORTFOLIO_MODEL_HPP
#define SPIRE_CONTROLLER_PORTFOLIO_MODEL_HPP
#include <unordered_map>
#include "Nexus/Accounting/PortfolioController.hpp"
#include "Nexus/Accounting/TrueAverageBookkeeper.hpp"
#include "Spire/Async/QtTaskQueue.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

namespace Spire {

  /** Maintains portfolio entries from a portfolio controller. */
  class ControllerPortfolioModel : public PortfolioModel {
    public:

      /** The controller supplying portfolio entries. */
      using Controller = Nexus::PortfolioController<
        Nexus::Portfolio<Nexus::TrueAverageBookkeeper>,
        Nexus::MarketDataClient>;

      /**
       * Constructs a ControllerPortfolioModel.
       * @param controller The controller whose portfolio to model.
       */
      explicit ControllerPortfolioModel(
        std::shared_ptr<Controller> controller);

      ~ControllerPortfolioModel() override;

      int get_size() const override;
      const Type& get(int index) const override;
      boost::signals2::connection connect_operation_signal(
        const OperationSignal::slot_type& slot) const override;
      void start_transaction() override;
      void end_transaction() override;

    private:
      std::shared_ptr<Controller> m_controller;
      ArrayListModel<Type> m_entries;
      std::unordered_map<Nexus::Ticker, int> m_indices;
      QtTaskQueue m_events;

      void on_update(const Nexus::PortfolioUpdateEntry& update);
  };
}

#endif
