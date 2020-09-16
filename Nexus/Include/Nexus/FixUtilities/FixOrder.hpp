#ifndef NEXUS_FIX_ORDER_HPP
#define NEXUS_FIX_ORDER_HPP
#include <type_traits>
#include <Beam/Threading/Sync.hpp>
#include <boost/lexical_cast.hpp>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include <quickfix/Message.h>
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/FixUtilities/FixUtilities.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::FixUtilities {
namespace Details {
  template<bool Enabled>
  struct AddAdditionalFix42Tag {
    template<typename Message>
    void operator ()(const Tag& tag, Beam::Out<Message> message) const {}
  };

  template<>
  struct AddAdditionalFix42Tag<true> {
    template<typename Message>
    void operator ()(const Tag& tag, Beam::Out<Message> message) const {
      if(tag.GetKey() == FIX::FIELD::PegDifference) {
        auto value = boost::get<Money>(tag.GetValue());
        auto pegDifference = FIX::PegDifference(static_cast<double>(value));
        message->set(pegDifference);
      }
    }
  };
}

  /**
   * Extends a PrimitiveOrder to store information useful when transmitting
   * Orders via the FIX protocol.
   */
  class FixOrder : public OrderExecutionService::PrimitiveOrder {
    public:

      /**
       * Constructs a FixOrder.
       * @param info The OrderInfo represented.
       * @param side The FIX Side used to execute this Order.
       */
      FixOrder(OrderExecutionService::OrderInfo info, FIX::Side side);

      /**
       * Constructs a FixOrder.
       * @param orderRecord The OrderRecord represented.
       * @param side The FIX Side used to execute this Order.
       */
      FixOrder(OrderExecutionService::OrderRecord orderRecord, FIX::Side side);

      /** Returns the FIX Symbol used to execute this Order. */
      const FIX::Symbol& GetSymbol() const;

      /** Returns the FIX Side used to execute this Order. */
      FIX::Side GetSide() const;

      /** Returns the next available cancel ID. */
      FIX::ClOrdID GetNextCancelId();

    private:
      FIX::Symbol m_symbol;
      FIX::Side m_side;
      Beam::Threading::Sync<int> m_cancelId;
  };

  //! Adds additional FIX tags to a FIX Message.
  /*!
    \param additionalTags The additional FIX tags to add.
    \param message The FIX Message to add the tags to.
  */
  template<typename NewMessageType>
  inline void AddAdditionalTags(const std::vector<Tag>& additionalTags,
      Beam::Out<NewMessageType> message) {
    for(const Tag& tag : additionalTags) {
      if(tag.GetKey() == FIX::FIELD::MaxFloor) {
        auto qty = boost::get<Quantity>(tag.GetValue());
        if(qty >= 0) {
          FIX::MaxFloor maxFloor(static_cast<FIX::QTY>(qty));
          message->set(maxFloor);
        }
      } else if(tag.GetKey() == FIX::FIELD::ExecInst) {
        auto value = boost::get<std::string>(tag.GetValue());
        FIX::ExecInst execInst(value);
        message->set(execInst);
      } else {
        Details::AddAdditionalFix42Tag<
          std::is_base_of<FIX42::Message, NewMessageType>::value>()(tag,
          Beam::Store(message));
      }
    }
  }

  inline FixOrder::FixOrder(OrderExecutionService::OrderInfo info,
      FIX::Side side)
      : OrderExecutionService::PrimitiveOrder(std::move(info)),
        m_side(side),
        m_cancelId(0) {
    m_symbol = GetInfo().m_fields.m_security.GetSymbol();
  }

  inline FixOrder::FixOrder(OrderExecutionService::OrderRecord orderRecord,
      FIX::Side side)
      : OrderExecutionService::PrimitiveOrder(std::move(orderRecord)),
        m_side(side),
        m_cancelId(0) {
    m_symbol = GetInfo().m_fields.m_security.GetSymbol();
  }

  inline const FIX::Symbol& FixOrder::GetSymbol() const {
    return m_symbol;
  }

  inline FIX::Side FixOrder::GetSide() const {
    return m_side;
  }

  inline FIX::ClOrdID FixOrder::GetNextCancelId() {
    auto cancelToken = std::string();
    Beam::Threading::With(m_cancelId,
      [&] (auto& cancelId) {
        cancelToken = boost::lexical_cast<std::string>(GetInfo().m_orderId) +
          "-" + boost::lexical_cast<std::string>(cancelId);
        ++cancelId;
      });
    return cancelToken;
  }
}

#endif
