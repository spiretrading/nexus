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
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Blotter/Blotter.hpp"
#include "Spire/Spire/Spire.hpp"

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
        Beam::Ref<const Nexus::OrderExecutionService::OrderExecutionPublisher>
        orderExecutionPublisher);

    private:
      struct OrderEntry {
        const Nexus::OrderExecutionService::Order* m_order;
        bool m_cancelSubmitted;
        Nexus::OrderStatus m_status;

        OrderEntry(const Nexus::OrderExecutionService::Order& order);
      };
      typedef std::array<std::deque<std::shared_ptr<OrderEntry>>,
        Nexus::Side::COUNT> SideToOrderEntryList;
      UserProfile* m_userProfile;
      const Nexus::OrderExecutionService::OrderExecutionPublisher*
        m_orderExecutionPublisher;
      std::unordered_map<Nexus::Security, SideToOrderEntryList>
        m_securityToOrderEntryList;
      std::optional<Beam::RoutineTaskQueue> m_slotHandler;

      void OnOrderExecuted(const Nexus::OrderExecutionService::Order* order);
      void OnExecutionReport(std::weak_ptr<OrderEntry> weakOrderEntry,
        const Nexus::OrderExecutionService::ExecutionReport& report);
  };
}

#endif
