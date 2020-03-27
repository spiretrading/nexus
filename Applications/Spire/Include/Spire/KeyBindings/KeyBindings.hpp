#ifndef SPIRE_KEY_BINDINGS_HPP
#define SPIRE_KEY_BINDINGS_HPP
#include <ostream>
#include <string>
#include <variant>
#include <vector>
#include <boost/optional.hpp>
#include <QHash>
#include <QKeySequence>
#include "Nexus/Definitions/Definitions.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Tag.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"

namespace Spire {
namespace Details {
  template<typename>
  struct ToVariantOfOptionals;

  template<template<typename...> class C, typename... T>
  struct ToVariantOfOptionals<C<T...>> {
    using Type = std::variant<boost::optional<T>...>;
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

        /*! \struct CustomTag
            \brief Represents a tag other than the standard order fields.
        */
        struct CustomTag {

          //! Specifies the types of values that can be stored by a CustomTag.
          using Type = Details::CustomTagType;

          //! The name of the tag.
          std::string m_name;

          //! The optional value of the tag.
          Type m_value;
        };

        //! The name of the action.
        std::string m_name;

        //! The type of the order.
        boost::optional<Nexus::OrderType> m_type;

        //! The side of the order.
        boost::optional<Nexus::Side> m_side;

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

      /*! \struct OrderActionBinding
          \brief Stores a key binding to an OrderAction.
      */
      struct OrderActionBinding {

        //! The key sequence the binding is mapped to.
        QKeySequence m_sequence;

        //! The region the action belongs to.
        Nexus::Region m_region;

        //! The action to perform.
        OrderAction m_action;
      };

      /*! \struct CancelActionBinding
          \brief Stores a key binding to a CancelAction.
      */
      struct CancelActionBinding {

        //! The key sequence the binding is mapped to.
        QKeySequence m_sequence;

        //! The region the action belongs to.
        Nexus::Region m_region;

        //! The action to perform.
        CancelAction m_action;

        bool operator ==(const CancelActionBinding& rhs) const;

        bool operator !=(const CancelActionBinding& rhs) const;
      };

      /*! \struct ActionBinding
          \brief Stores a key binding to an Action.
      */
      struct ActionBinding {

        //! The key sequence the binding is mapped to.
        QKeySequence m_sequence;

        //! The region the action belongs to.
        Nexus::Region m_region;

        //! The action to perform.
        Action m_action;
      };

      static KeyBindings get_default_key_bindings();

      //! Associates a key sequence with an action for a region.
      /*!
        \param sequence The key sequence.
        \param region The region.
        \param action The action.
      */
      void set(QKeySequence sequence, const Nexus::Region& region,
        const Action& action);

      //! Removes a binding for a key sequence within the provided region.
      /*!
        \param region The region the binding is set for.
        \param sequence The key sequence.
        \detail The region should be Global for cancellation actions.
      */
      void reset(const Nexus::Region& region, const QKeySequence& sequence);

      //! Returns a action associated with a key sequence within a region.
      /*!
        \param region The region to look for a binding in.
        \param sequence The key sequence.
        \return The action in the binding exists.
      */
      boost::optional<Action> find(const Nexus::Region& region,
        const QKeySequence& sequence) const;

      //! Lists all order action bindings.
      /*!
        \return The list of all order action bindings.
      */
      std::vector<OrderActionBinding> build_order_bindings() const;

      //! Lists all cancel action bindings.
      /*!
        \return The list of all cancel action bindings.
      */
      std::vector<CancelActionBinding> build_cancel_bindings() const;

      //! Lists all bindings.
      /*!
        \return The list of all bindings.
      */
      std::vector<ActionBinding> build_action_bindings() const;

    private:
      using Actions = Nexus::RegionMap<boost::optional<Action>>;
      QHash<QKeySequence, Actions> m_bindings;
  };

  std::ostream& operator <<(std::ostream& out,
    const KeyBindings::OrderAction& action);

  std::ostream& operator <<(std::ostream& out,
    const KeyBindings::OrderAction::CustomTag& tag);

  std::ostream& operator <<(std::ostream& out,
    KeyBindings::CancelAction action);
}

#endif
