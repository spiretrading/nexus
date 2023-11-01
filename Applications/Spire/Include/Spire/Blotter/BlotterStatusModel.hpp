#ifndef SPIRE_BLOTTER_STATUS_MODEL_HPP
#define SPIRE_BLOTTER_STATUS_MODEL_HPP
#include <memory>
#include <unordered_map>
#include "Nexus/Definitions/ExchangeRateTable.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Blotter/PortfolioModel.hpp"
#include "Spire/Ui/MoneyBox.hpp"

namespace Spire {

  /** Models a blotter's overall status (as displayed in the status bar). */
  class BlotterStatusModel {
    public:

      /**
       * Constructs a BlotterStatusModel from an account's buying power and a
       * portfolio model representing a single blotter.
       * @param currency The currency to express profit and loss totals in.
       * @param exchange_rates The table of exchange rates used to convert all
       *        profit and loss calculations to the specified <i>currency</i>.
       * @param buying_power The represented account's buying power.
       * @param net_loss The represented account's net loss.
       * @param portfolio The represented blotter's portfolio.
       */
      BlotterStatusModel(Nexus::CurrencyId currency,
        Nexus::ExchangeRateTable exchange_rates,
        std::shared_ptr<MoneyModel> buying_power,
        std::shared_ptr<MoneyModel> net_loss,
        std::shared_ptr<PortfolioModel> portfolio);

      /** Returns the account's available buying power. */
      const std::shared_ptr<MoneyModel>& get_buying_power();

      /** Returns the account's net loss. */
      const std::shared_ptr<MoneyModel>& get_net_loss();

      /** Returns the blotter's cost basis. */
      const std::shared_ptr<MoneyModel>& get_cost_basis();

      /** Returns the blotter's unrealized profit and loss. */
      const std::shared_ptr<MoneyModel>& get_unrealized_profit_and_loss();

      /** Returns the blotter's fees. */
      const std::shared_ptr<MoneyModel>& get_fees();

      /** Returns the blotter's realized profit and loss. */
      const std::shared_ptr<MoneyModel>& get_realized_profit_and_loss();

      /** Returns the blotter's total profit and loss. */
      const std::shared_ptr<MoneyModel>& get_total_profit_and_loss();

    private:
      Nexus::CurrencyId m_currency;
      Nexus::ExchangeRateTable m_exchange_rates;
      std::shared_ptr<MoneyModel> m_buying_power;
      std::shared_ptr<MoneyModel> m_net_loss;
      std::shared_ptr<PortfolioModel> m_portfolio;
      boost::signals2::scoped_connection m_portfolio_connection;
      std::unordered_map<Nexus::CurrencyId, Nexus::Money> m_cost_basis_totals;
      std::shared_ptr<MoneyModel> m_cost_basis;
      std::unordered_map<Nexus::CurrencyId, Nexus::Money> m_unrealized_totals;
      std::shared_ptr<MoneyModel> m_unrealized_profit_and_loss;
      std::unordered_map<Nexus::CurrencyId, Nexus::Money> m_fee_totals;
      std::shared_ptr<MoneyModel> m_fees;
      std::unordered_map<Nexus::CurrencyId, Nexus::Money> m_realized_totals;
      std::shared_ptr<MoneyModel> m_realized_profit_and_loss;
      std::shared_ptr<MoneyModel> m_total_profit_and_loss;

      BlotterStatusModel(const BlotterStatusModel&) = delete;
      BlotterStatusModel& operator =(const BlotterStatusModel&) = delete;
      void on_portfolio(const PortfolioModel::Portfolio::UpdateEntry& update);
  };
}

#endif
