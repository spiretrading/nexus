#ifndef SPIRE_CANCELONFILLCONTROLLER_HPP
#define SPIRE_CANCELONFILLCONTROLLER_HPP
#include <array>
#include <deque>
#include <optional>
#include <unordered_map>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class CancelOnFillController
      \brief Implements the cancel on fill interactions property.
   */
  class CancelOnFillController : private boost::noncopyable {
    public:

      //! Constructs a CancelOnFillController.
      /*!
        \param userProfile The user's profile.
      */
      CancelOnFillController(Beam::Ref<UserProfile> userProfile);

      //! Sets the OrderExecutionPublisher whose Orders are checked for fills.
      /*!
        \param orderExecutionPublisher The OrderExecutionPublisher whose Orders
               are checked for fills.
      */
      void SetOrderExecutionPublisher(
        Beam::Ref<const Beam::Publisher<std::shared_ptr<Nexus::Order>>>
          orderExecutionPublisher);

    private:
      struct OrderEntry {
        std::shared_ptr<Nexus::Order> m_order;
        bool m_cancelSubmitted;
        Nexus::OrderStatus m_status;

        OrderEntry(const std::shared_ptr<Nexus::Order>& order);
      };
      typedef std::array<std::deque<std::shared_ptr<OrderEntry>>,
        Nexus::Side::COUNT> SideToOrderEntryList;
      UserProfile* m_userProfile;
      const Beam::Publisher<std::shared_ptr<Nexus::Order>>*
        m_orderExecutionPublisher;
      std::unordered_map<Nexus::Ticker, SideToOrderEntryList>
        m_tickerToOrderEntryList;
      std::optional<Beam::RoutineTaskQueue> m_slotHandler;

      void OnOrderExecuted(const std::shared_ptr<Nexus::Order>& order);
      void OnExecutionReport(std::weak_ptr<OrderEntry> weakOrderEntry,
        const Nexus::ExecutionReport& report);
  };
}

#endif
