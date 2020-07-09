#ifndef SPIRE_KEY_BINDINGS_HPP
#define SPIRE_KEY_BINDINGS_HPP
#include <ostream>
#include <string>
#include <variant>
#include <vector>
#include <boost/optional.hpp>
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

  template<typename...>
  struct AddOptionalToVariant;

  template<template<typename...> class C, typename T, typename... T0>
  struct AddOptionalToVariant<C<T0...>, T> {
    using Type = std::variant<T0..., boost::optional<T>>;
  };
}

  //! Stores the user's key bindings.
  class KeyBindings {
    public:

      //! Represents a tag with an optional value that can be passed to an
      //! order action.
      class Tag {
        public:

          //! The variant of all possible types of tag values.
          using Type = typename Details::AddOptionalToVariant<
            typename Details::AddOptionalToVariant<
            typename Details::AddOptionalToVariant<
            typename Details::AddOptionalToVariant<
            typename Details::ToVariantOfOptionals<Nexus::Tag::Type>::Type,
            Nexus::Side>::Type, Nexus::OrderType>::Type,
            Nexus::Region>::Type, Nexus::TimeInForce>::Type;

          //! Constructs a Tag.
          /*!
            \param key The key of the tag.
            \param value The value of the tag.
          */
          template<typename T>
          Tag(int key, T value);

          //! Constructs a Tag.
          /*!
            \param key The key of the tag.
            \param value The value of the tag.
          */
          template<typename T>
          Tag(int key, boost::optional<T> value);

          //! Returns the key of the tag.
          int get_key() const;

          //! Returns the value of the tag.
          /*!
            \tparam T The type of tag's value.
            \return The value of the tag, boost::none if the type of the value
                    is not T.
          */
          template<typename T>
          boost::optional<T> get_value() const;

          //! Returns the value of the tag.
          /*!
            \return The value of the tag contained in a variant of all possible
                    types.
          */
          Type get_value() const;

          //! Checks whether two tags are equal.
          bool operator ==(const Tag& other) const;

          //! Checks whether two tags are not equal.
          bool operator !=(const Tag& other) const;

        private:
          friend std::ostream& operator <<(std::ostream& out,
            const KeyBindings::Tag& tag);

          int m_key;
          Type m_value;
      };

      //! Stores a description of an order submission action.
      class OrderAction {
        public:

          //! Constructs an unnamed OrderAction with no tags.
          OrderAction() = default;

          //! Constructs an OrderAction.
          /*!
            \param name The name of the action.
            \param tags The list of the tags.
          */
          OrderAction(std::string name, std::vector<Tag> tags);

          //! Returns the name of the action.
          std::string get_name() const;

          //! Renames the action.
          /*!
            \param name The new name of the action.
          */
          void set_name(std::string name);

          //! Returns the tag with the key provided.
          /*!
            \return The tag if present, boost::none otherwise.
          */
          boost::optional<Tag> get_tag(int tag_key) const;

          //! Adds a new tag to the list, or updates a value of the existing
          //! tag if one with the same key is already stored.
          /*!
            \param tag The tag to set.
          */
          void set_tag(Tag tag);

          //! Returns a list of all tags.
          std::vector<Tag> get_tags() const;

        private:
          std::string m_name;
          std::vector<Tag> m_tags;
      };

      //! Represents an order cancellation action.
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

      //! Stores a key binding to an OrderAction.
      struct OrderActionBinding {

        //! The key sequence the binding is mapped to.
        QKeySequence m_sequence;

        //! The region the action belongs to.
        Nexus::Region m_region;

        //! The action to perform.
        OrderAction m_action;
      };

      //! Stores a key binding to a CancelAction.
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

      //! Stores a key binding to an Action.
      struct ActionBinding {

        //! The key sequence the binding is mapped to.
        QKeySequence m_sequence;

        //! The region the action belongs to.
        Nexus::Region m_region;

        //! The action to perform.
        Action m_action;
      };

      //! Returns the default key bindings.
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
      struct KeyBindingMapping {
        QKeySequence m_key_sequence;
        Actions m_actions;
      };

      std::vector<KeyBindingMapping> m_bindings;
  };

  template<typename T>
  KeyBindings::Tag::Tag(int key, T value)
    : Tag(key, boost::optional<T>(std::move(value))) {}

  template<typename T>
  KeyBindings::Tag::Tag(int key, boost::optional<T> value)
    : m_key(key),
      m_value(std::move(value)) {}

  template<typename T>
  boost::optional<T> KeyBindings::Tag::get_value() const {
    return std::visit(
      [] (auto& value) {
        if constexpr(std::is_same_v<std::decay_t<decltype(value)>,
            boost::optional<T>>) {
          return value;
        } else {
          return boost::optional<T>();
        }
      }, m_value);
  }

  std::ostream& operator <<(std::ostream& out,
    const boost::optional<Nexus::Region>& region);

  std::ostream& operator <<(std::ostream& out,
    const KeyBindings::Action& action);

  std::ostream& operator <<(std::ostream& out,
    const KeyBindings::OrderAction& action);

  std::ostream& operator <<(std::ostream& out,
    const KeyBindings::Tag& tag);

  std::ostream& operator <<(std::ostream& out,
    KeyBindings::CancelAction action);
}

#endif
