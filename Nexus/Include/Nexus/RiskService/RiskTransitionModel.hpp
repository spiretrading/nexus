#ifndef NEXUS_RISK_TRANSITION_MODEL_HPP
#define NEXUS_RISK_TRANSITION_MODEL_HPP
#include <iostream>
#include <unordered_set>
#include <vector>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/ReportException.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Accounting/PositionOrderBook.hpp"
#include "Nexus/Definitions/Destination.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"
#include "Nexus/RiskService/RiskState.hpp"

namespace Nexus {

  /**
   * Keeps track of an account's RiskState and performs the actions required to
   * transition from one RiskState to another.
   * @param <C> The type of OrderExecutionClient used to cancel Orders and
   *        flatten Positions.
   */
  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  class RiskTransitionModel {
    public:

      /**
       * The type of OrderExecutionClient used to cancel Orders and flatten
       * Positions.
       */
      using OrderExecutionClient = Beam::dereference_t<C>;

      /**
       * Constructs a RiskTransitionModel.
       * @param account The account being tracked.
       * @param inventory A snapshot of the account's positions.
       * @param risk_state The account's initial RiskState.
       * @param order_execution_client The OrderExecutionClient used to cancel
       *        Orders and flatten Positions.
       * @param destinations The database of destinations used to flatten
       *        Orders.
       */
      template<Beam::Initializes<C> CF>
      RiskTransitionModel(Beam::DirectoryEntry account,
        const std::vector<Inventory>& inventory, RiskState risk_state,
        CF&& order_execution_client, DestinationDatabase destinations) noexcept;

      /**
       * Adds an Order.
       * @param order The Order to add.
       */
      void add(const std::shared_ptr<Order>& order);

      /**
       * Updates the RiskState.
       * @param state The new RiskState.
       */
      void update(const RiskState& state);

      /**
       * Updates an Order with an ExecutionReport.
       * @param report The ExecutionReport containing the details of the update.
       */
      void update(const ExecutionReport& report);

    private:
      Beam::DirectoryEntry m_account;
      RiskState m_risk_state;
      Beam::local_ptr_t<C> m_order_execution_client;
      DestinationDatabase m_destinations;
      PositionOrderBook m_book;
      std::unordered_set<OrderId> m_live_orders;
      int m_state;

      bool c0();
      bool c1();
      bool c2();
      bool c3();
      void s0();
      void s1();
      void s2();
      void s3();
      void s4();
      void s5();
      void s6();
  };

  template<typename C>
  RiskTransitionModel(Beam::DirectoryEntry,
    const std::vector<Inventory>&, RiskState, C&&, DestinationDatabase) ->
      RiskTransitionModel<std::remove_cvref_t<C>>;

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  RiskTransitionModel<C>::RiskTransitionModel(
    Beam::DirectoryEntry account,
    const std::vector<Inventory>& inventory, RiskState risk_state,
    CF&& order_execution_client, DestinationDatabase destinations) noexcept
    : m_account(std::move(account)),
      m_risk_state(std::move(risk_state)),
      m_order_execution_client(std::forward<CF>(order_execution_client)),
      m_destinations(std::move(destinations)),
      m_book(inventory),
      m_state(0) {}

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::add(const std::shared_ptr<Order>& order) {
    m_book.add(order);
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::update(const RiskState& state) {
    m_risk_state = state;
    if(m_state == 0) {
      return s0();
    } else if(m_state == 2) {
      return s2();
    } else if(m_state == 4) {
      return s4();
    } else if(m_state == 6) {
      return s6();
    }
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::update(const ExecutionReport& report) {
    m_book.update(report);
    if(is_terminal(report.m_status)) {
      m_live_orders.erase(report.m_id);
    }
    if(m_state == 4) {
      return s4();
    }
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  bool RiskTransitionModel<C>::c0() {
    return m_risk_state.m_type == RiskState::Type::CLOSE_ORDERS;
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  bool RiskTransitionModel<C>::c1() {
    return m_risk_state.m_type == RiskState::Type::ACTIVE;
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  bool RiskTransitionModel<C>::c2() {
    return m_risk_state.m_type == RiskState::Type::DISABLED;
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  bool RiskTransitionModel<C>::c3() {
    return m_live_orders.empty();
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s0() {
    m_state = 0;
    if(c0()) {
      return s1();
    }
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s1() {
    m_state = 1;
    for(auto& order : m_book.get_opening_orders()) {
      m_order_execution_client->cancel(*order);
    }
    return s2();
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s2() {
    m_state = 2;
    if(c1()) {
      return s0();
    } else if(c2()) {
      return s3();
    }
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s3() {
    m_state = 3;
    auto& live_orders = m_book.get_live_orders();
    m_live_orders.clear();
    for(auto& order : live_orders) {
      m_live_orders.insert(order->get_info().m_id);
      m_order_execution_client->cancel(*order);
    }
    return s4();
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s4() {
    m_state = 4;
    if(c1()) {
      return s0();
    } else if(c3()) {
      return s5();
    }
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s5() {
    m_state = 5;
    for(auto& position : m_book.get_positions()) {
      auto destination = m_destinations.get_preferred_destination(
        position.m_security.get_venue()).m_id;
      auto fields = make_market_order_fields(m_account, position.m_security,
        get_opposite(get_side(position.m_quantity)), destination,
        abs(position.m_quantity));
      try {
        m_order_execution_client->submit(fields);
      } catch(const std::exception&) {
        std::cout << BEAM_REPORT_CURRENT_EXCEPTION() << std::flush;
      }
    }
    return s6();
  }

  template<typename C> requires IsOrderExecutionClient<Beam::dereference_t<C>>
  void RiskTransitionModel<C>::s6() {
    m_state = 6;
    if(c1()) {
      return s0();
    }
  }
}

#endif
