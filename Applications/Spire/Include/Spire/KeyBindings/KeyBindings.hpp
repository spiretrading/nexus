#ifndef SPIRE_KEY_BINDINGS_HPP
#define SPIRE_KEY_BINDINGS_HPP
#include <string>
#include <variant>
#include <vector>
#include <boost/optional/optional.hpp>
#include <Nexus/Definitions/Definitions.hpp>
#include <Nexus/Definitions/OrderType.hpp>
#include <Nexus/Definitions/Region.hpp>
#include <Nexus/Definitions/Side.hpp>
#include <Nexus/Definitions/Tag.hpp>
#include <QKeySequence>

namespace Spire {
namespace Details {

  template<typename>
  struct ToVariantOfOptionals;

  template<template<typename...> class C, typename... T>
  struct ToVariantOfOptionals<C<T...>> {
    using Type = std::variant<T...>;
  };

  using CustomTagType =
    typename ToVariantOfOptionals<Nexus::Tag::Type>::Type;
}

  /*! \class KeyBindings
      \brief Stores the user's key bindings.
  */
  class KeyBindings {
    public:

      /*! \struct OrderAction
          \brief Stores a description of an order submission action.
      */
      struct OrderAction {

        struct CustomTag {

          //! Specifies the types of values that can be stored by a CustomTag.
          using Type = Details::CustomTagType;

          //! The name of the tag.
          std::string m_name;

          //! The optional value of the tag.
          boost::optional<Type> m_value;
        };

        //! The name of the action.
        std::string m_name;

        //! The type of the order.
        boost::optional<Nexus::OrderType> m_type;

        //! The side of the order.
        boost::optional<Nexus::Side> m_side;

        //! The destination of the order.
        Nexus::Region m_region;

        //! The expiry of the order.
        boost::optional<Nexus::TimeInForce> m_time_in_force;

        //! The quantity of the order.
        boost::optional<Nexus::Quantity> m_quantity;

        //! The list of custom tags.
        std::vector<CustomTag> m_tags;
      };

      /*! \enum CancelAction
          \brief Represents an order cancellation action.
      */
      enum class CancelAction {

        //! Cancels the most recent OrderAction.
        MOST_RECENT,

        //! Cancels the most recent ask OrderAction.
        MOST_RECENT_ASK,

        //! Cancels the most recent bid OrderAction.
        MOST_RECENT_BID,

        //! Cancels the oldest OrderAction.
        OLDEST,

        //! Cancels the oldest ask OrderAction.
        OLDEST_ASK,

        //! Cancels the oldest bid OrderAction.
        OLDEST_BID,

        //! Cancels all Actions.
        ALL,

        //! Cancels all asks.
        ALL_ASKS,

        //! Cancels all bids.
        ALL_BIDS,

        //! Cancels the closest ask by price then time.
        CLOSEST_ASK,

        //! Cancels the closest bid by price then time.
        CLOSEST_BID,

        //! Cancels the furthest ask by price then time.
        FURTHEST_ASK,

        //! Cancels the furthest bid by price then time.
        FURTHEST_BID
      };

      //! The type of any action supported by bindings.
      using Action = std::variant<OrderAction, CancelAction>;

      //! the type of the result of ListOrderBindings query.
      using OrderBindingsList = std::vector<std::pair<QKeySequence,
        OrderAction>>;

      //! The type of the result of ListCancelBindings query.
      using CancelBindingsList = std::vector<std::pair<QKeySequence,
        CancelAction>>;

      //! The type of the result of ListActionBindings query.
      using ActionBindingsList = std::vector<std::pair<QKeySequence,
        Action>>;

      //! Associates a key sequence with a action.
      /*!
        \param sequence The key sequence.
        \param action The action.
      */
      void set_binding(QKeySequence sequence, Action action);

      //! Removes a binding for a key sequence within the provided region.
      /*!
        \param region The region the binding is set for.
        \param sequence The key sequence.
        \detail The region should be Global for cancellation actions.
      */
      void reset_binding(const Nexus::Region& region,
        const QKeySequence& sequence);

      //! Returns a action associated with a key sequence within a region.
      /*!
        \param region The region to look for a binding in.
        \param sequence The key sequence.
        \return Reference to a action if the binding exists,
                boost::none otherwise.
      */
      boost::optional<const Action&> find_binding(const Nexus::Region& region,
        const QKeySequence& sequence) const;

      //! Lists all order action bindings.
      /*!
        \return The list of all order action bindings.
      */
      OrderBindingsList build_order_bindings_list() const;
      
      //! Lists all cancel action bindings.
      /*!
        \return The list of all cancel action bindings.
      */
      CancelBindingsList build_cancel_bindings_list() const;

      //! Lists all bindings.
      /*!
        \return The list of all bindings.
      */
      ActionBindingsList build_action_bindings_list() const;
  };
}

#endif
