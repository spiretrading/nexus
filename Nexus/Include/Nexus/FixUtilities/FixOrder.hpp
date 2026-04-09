#ifndef NEXUS_FIX_ORDER_HPP
#define NEXUS_FIX_ORDER_HPP
#include <atomic>
#include <type_traits>
#include <quickfix/FixFields.h>
#include <quickfix/FixValues.h>
#include <quickfix/fix42/Message.h>
#include "Nexus/FixUtilities/FixConversions.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
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
      if(tag.get_key() == FIX::FIELD::PegDifference) {
        auto value = boost::get<Money>(tag.get_value());
        auto peg_difference = FIX::PegDifference(static_cast<double>(value));
        message->set(peg_difference);
      }
    }
  };
}

  /**
   * Extends a PrimitiveOrder to store information useful when transmitting
   * Orders via the FIX protocol.
   */
  class FixOrder : public PrimitiveOrder {
    public:

      /**
       * Constructs a FixOrder.
       * @param info The OrderInfo represented.
       * @param side The FIX Side used to execute this Order.
       */
      FixOrder(OrderInfo info, FIX::Side side);

      /**
       * Constructs a FixOrder.
       * @param record The OrderRecord represented.
       * @param side The FIX Side used to execute this Order.
       */
      FixOrder(OrderRecord record, FIX::Side side);

      /** Returns the FIX Symbol used to execute this Order. */
      const FIX::Symbol& get_symbol() const;

      /** Returns the FIX Side used to execute this Order. */
      FIX::Side get_side() const;

      /** Returns the next available cancel ID. */
      FIX::ClOrdID get_next_cancel_id();

    private:
      FIX::Symbol m_symbol;
      FIX::Side m_side;
      std::atomic_int m_cancel_id;
  };

  /**
   * Adds additional FIX tags to a FIX Message.
   * @param additional_tags The additional FIX tags to add.
   * @param message The FIX Message to add the tags to.
   */
  template<typename NewMessageType>
  void add_additional_tags(const std::vector<Tag>& additional_tags,
      Beam::Out<NewMessageType> message) {
    for(auto& tag : additional_tags) {
      if(tag.get_key() == FIX::FIELD::MaxFloor) {
        auto qty = boost::get<Quantity>(tag.get_value());
        if(qty >= 0) {
          message->set(FIX::MaxFloor(static_cast<FIX::QTY>(qty)));
        }
      } else if(tag.get_key() == FIX::FIELD::ExecInst) {
        auto value = boost::get<std::string>(tag.get_value());
        message->set(FIX::ExecInst(value));
      } else {
        Details::AddAdditionalFix42Tag<
          std::is_base_of_v<FIX42::Message, NewMessageType>>()(
            tag, Beam::out(message));
      }
    }
  }

  inline FixOrder::FixOrder(OrderInfo info, FIX::Side side)
      : PrimitiveOrder(std::move(info)),
        m_side(side),
        m_cancel_id(0) {
    m_symbol = get_info().m_fields.m_ticker.get_symbol();
  }

  inline FixOrder::FixOrder(OrderRecord record, FIX::Side side)
      : PrimitiveOrder(std::move(record)),
        m_side(side),
        m_cancel_id(0) {
    m_symbol = get_info().m_fields.m_ticker.get_symbol();
  }

  inline const FIX::Symbol& FixOrder::get_symbol() const {
    return m_symbol;
  }

  inline FIX::Side FixOrder::get_side() const {
    return m_side;
  }

  inline FIX::ClOrdID FixOrder::get_next_cancel_id() {
    auto id = ++m_cancel_id;
    auto cancel_token =
      std::to_string(get_info().m_id) + '-' + std::to_string(id);
    return cancel_token;
  }
}

#endif
