#ifndef SPIRE_KEYBINDINGS_HPP
#define SPIRE_KEYBINDINGS_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <Beam/Serialization/ShuttleUnorderedMap.hpp>
#include <boost/optional/optional.hpp>
#include <QKeySequence>
#include "Nexus/Definitions/Venue.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/UI/HashQtTypes.hpp"
#include "Spire/UI/UI.hpp"

class QKeyEvent;

namespace Spire {
namespace Details {

  /*! \enum KeyBindingsCancelBindingType
      \brief Lists the types of cancels.
   */
  BEAM_ENUM(KeyBindingsCancelBindingTypeDefinition,

    //! Cancels the most recent Task.
    MOST_RECENT,

    //! Cancels the most recent ask Task.
    MOST_RECENT_ASK,

    //! Cancels the most recent bid Task.
    MOST_RECENT_BID,

    //! Cancels the oldest Task.
    OLDEST,

    //! Cancels the oldest ask Task.
    OLDEST_ASK,

    //! Cancels the oldest bid Task.
    OLDEST_BID,

    //! Cancels all Tasks.
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
    FURTHEST_BID);
}

  /*! \enum KeyModifiers
      \brief Lists the types of key modifiers.
   */
  BEAM_ENUM(KeyModifiers,

    //! No modifier.
    PLAIN,

    //! Shift key.
    SHIFT,

    //! Alt key.
    ALT,

    //! Control key.
    CONTROL);

  /*! \class KeyBindings
      \brief Stores the user's key bindings.
   */
  class KeyBindings {
    public:

      /*! \struct TaskBinding
          \brief Stores a key binding to a Task.
       */
      struct TaskBinding {

        //! Returns the list of all keys available for TaskBindings.
        static const std::vector<QKeySequence>& GetAvailableKeys();

        //! The name of the TaskBinding.
        std::string m_name;

        //! The CanvasNode the key binds to.
        std::shared_ptr<CanvasNode> m_node;

        //! Constructs a TaskBinding.
        TaskBinding() = default;

        //! Constructs a TaskBinding.
        /*!
          \param name The name of the TaskBinding.
          \param node The CanvasNode the key binds to.
        */
        TaskBinding(std::string name, std::shared_ptr<CanvasNode> node);
      };

      /*! \struct CancelBinding
          \brief Stores a key binding to a type of Task cancellation.
       */
      struct CancelBinding {
        using Type = Details::KeyBindingsCancelBindingTypeDefinition;

        //! Returns the list of all keys available for CancelBindings.
        static const std::vector<QKeySequence>& GetAvailableKeys();

        //! Returns a CancelBinding from a Type.
        static CancelBinding GetCancelBindingFromType(Type type);

        //! Performs the steps needed to carry out a CancelBinding.
        /*!
          \param cancelBinding The type of cancel to perform.
          \param tasks The Tasks to cancel.
        */
        static void HandleCancel(const CancelBinding& cancelBinding,
          Beam::Out<std::vector<std::shared_ptr<Task>>> tasks);

        //! Performs the steps needed to carry out a CancelBinding.
        /*!
          \param cancelBinding The type of cancel to perform.
          \param orderExecutionClient The OrderExecutionClient to used to
                 cancel the Orders.
          \param orders The Orders to cancel.
        */
        static void HandleCancel(const CancelBinding& cancelBinding,
          Nexus::OrderExecutionClient& orderExecutionClient,
          Beam::Out<std::vector<OrderLogModel::OrderEntry>> orders);

        //! Describes the type of cancel performed by this binding.
        std::string m_description;

        //! The type of cancel to perform.
        Type m_type;

        //! Constructs a CancelBinding.
        CancelBinding() = default;

        //! Constructs a CancelBinding.
        /*!
          \param description The cancel description.
          \param type The type of cancel to perform.
        */
        CancelBinding(std::string description, Type type);
      };

      //! Loads the KeyBindings from a UserProfile.
      /*!
        \param userProfile The UserProfile to load the properties from.
      */
      static void Load(Beam::Out<UserProfile> userProfile);

      //! Saves a UserProfile's KeyBindings.
      /*!
        \param userProfile The UserProfile's properties to save.
      */
      static void Save(const UserProfile& userProfile);

      //! Constructs KeyBindings.
      KeyBindings() = default;

      //! Returns the Task's CatalogEntry for a key binding.
      /*!
        \param venue The Venue context.
        \param binding The key binding to lookup.
        \return The TaskBinding for the specified <i>binding</i> within the
                context of the specified <i>venue</i>.
      */
      boost::optional<const TaskBinding&> GetTaskFromBinding(
        Nexus::Venue venue, const QKeySequence& binding) const;

      //! Resets a Task's CatalogEntry key binding.
      /*
        \param venue The Venue context.
        \param binding The key binding to associate.
      */
      void ResetTaskBinding(Nexus::Venue venue, const QKeySequence& binding);

      //! Sets a key binding for a Task.
      /*
        \param venue The Venue context.
        \param binding The key binding to associate.
        \param taskBinding The TaskBinding to associate.
      */
      void SetTaskBinding(Nexus::Venue venue, const QKeySequence& binding,
        const TaskBinding& taskBinding);

      //! Returns the CancelBinding associated with a key.
      /*!
        \param binding The key binding to lookup.
        \return The CancelBinding for the specified <i>binding</i>.
      */
      boost::optional<const CancelBinding&> GetCancelFromBinding(
        const QKeySequence& binding) const;

      //! Resets a CancelBinding.
      /*
        \param binding The key binding to reset.
      */
      void ResetCancelBinding(const QKeySequence& binding);

      //! Sets a CancelBinding.
      /*
        \param binding The key binding to associate.
        \param cancelBinding The CancelBinding to associate.
      */
      void SetCancelBinding(const QKeySequence& binding,
        const CancelBinding& cancelBinding);

      //! Returns the default Order quantity in a specified venue.
      /*!
        \param venue The venue to lookup.
        \return The default Order quantity in the specified venue.
      */
      Nexus::Quantity GetDefaultQuantity(Nexus::Venue venue) const;

      //! Sets the default Order quantity in a specified venue.
      /*!
        \param venue The venue to specify the default quantity for.
        \param quantity The default quantity.
      */
      void SetDefaultQuantity(Nexus::Venue venue, Nexus::Quantity quantity);

    private:
      friend struct Beam::DataShuttle;
      std::unordered_map<Nexus::Venue,
        std::unordered_map<QKeySequence, TaskBinding>> m_taskBindings;
      std::unordered_map<QKeySequence, CancelBinding> m_cancelBindings;
      std::unordered_map<Nexus::Venue, Nexus::Quantity> m_defaultQuantities;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void KeyBindings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("task_bindings", m_taskBindings);
    shuttle.shuttle("cancel_bindings", m_cancelBindings);
    shuttle.shuttle("default_quantities", m_defaultQuantities);
  }

  //! Returns the set of key modifiers represented by a key event.
  /*!
    \param event The event to inspect.
    \return The set of key modifiers represented by the <i>event</i>.
  */
  KeyModifiers KeyModifiersFromEvent(const QKeyEvent& event);
}

namespace Beam {
  template<>
  struct Shuttle<Spire::KeyBindings::TaskBinding> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::KeyBindings::TaskBinding& value,
        unsigned int version) const {
      shuttle.shuttle("name", value.m_name);
      shuttle.shuttle("node", value.m_node);
    }
  };

  template<>
  struct Shuttle<Spire::KeyBindings::CancelBinding> {
    template<IsShuttle S>
    void operator ()(S& shuttle,
        Spire::KeyBindings::CancelBinding& value, unsigned int version) const {
      shuttle.shuttle("description", value.m_description);
      shuttle.shuttle("type", value.m_type);
    }
  };
}

#endif
