#ifndef SPIRE_PORTFOLIO_SUMMARY_MODEL_HPP
#define SPIRE_PORTFOLIO_SUMMARY_MODEL_HPP
#include <unordered_map>
#include <vector>
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  /** Models a portfolio's currency totals in a reporting currency. */
  class PortfolioSummaryModel : public ValueModel<Nexus::PortfolioUpdateEntry> {
    public:

      /**
       * Constructs a PortfolioSummaryModel.
       * @param portfolio The portfolio to summarize.
       * @param currency The reporting currency.
       * @param rates Fixed exchange rates covering the portfolio currencies.
       */
      PortfolioSummaryModel(std::shared_ptr<PortfolioModel> portfolio,
        Nexus::CurrencyId currency, const Nexus::ExchangeRateTable& rates);

      const Type& get() const override;
      boost::signals2::connection connect_update_signal(
        const UpdateSignal::slot_type& slot) const override;

    private:
      mutable UpdateSignal m_update_signal;
      std::shared_ptr<PortfolioModel> m_portfolio;
      Nexus::ExchangeRateTable m_rates;
      Type m_value;
      std::vector<Nexus::Inventory> m_totals;
      std::unordered_map<Nexus::CurrencyId, Nexus::Money> m_unrealized;
      int m_depth;
      bool m_is_rebuild;
      boost::signals2::scoped_connection m_connection;

      void rebuild();
      void update();
      void on_operation(const PortfolioModel::Operation& operation);
  };
}

#endif
