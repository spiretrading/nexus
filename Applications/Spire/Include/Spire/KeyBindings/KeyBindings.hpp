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
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/HashQtTypes.hpp"

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
          Nexus::OrderExecutionService::OrderExecutionClientBox&
            orderExecutionClient,
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
        \param market The Market context.
        \param binding The key binding to lookup.
        \return The TaskBinding for the specified <i>binding</i> within the
                context of the specified <i>market</i>.
      */
      boost::optional<const TaskBinding&> GetTaskFromBinding(
        Nexus::MarketCode market, const QKeySequence& binding) const;

      //! Resets a Task's CatalogEntry key binding.
      /*
        \param market The Market context.
        \param binding The key binding to associate.
      */
      void ResetTaskBinding(Nexus::MarketCode market,
        const QKeySequence& binding);

      //! Sets a key binding for a Task.
      /*
        \param market The Market context.
        \param binding The key binding to associate.
        \param taskBinding The TaskBinding to associate.
      */
      void SetTaskBinding(Nexus::MarketCode market, const QKeySequence& binding,
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

      //! Returns the default Order quantity in a specified market.
      /*!
        \param market The market to lookup.
        \return The default Order quantity in the specified market.
      */
      Nexus::Quantity GetDefaultQuantity(Nexus::MarketCode market) const;

      //! Sets the default Order quantity in a specified market.
      /*!
        \param market The market to specify the default quantity for.
        \param quantity The default quantity.
      */
      void SetDefaultQuantity(Nexus::MarketCode market,
        Nexus::Quantity quantity);

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::unordered_map<Nexus::MarketCode,
        std::unordered_map<QKeySequence, TaskBinding>> m_taskBindings;
      std::unordered_map<QKeySequence, CancelBinding> m_cancelBindings;
      std::unordered_map<Nexus::MarketCode, Nexus::Quantity>
        m_defaultQuantities;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void KeyBindings::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("task_bindings", m_taskBindings);
    shuttle.Shuttle("cancel_bindings", m_cancelBindings);
    shuttle.Shuttle("default_quantities", m_defaultQuantities);
  }

  //! Returns the set of key modifiers represented by a key event.
  /*!
    \param event The event to inspect.
    \return The set of key modifiers represented by the <i>event</i>.
  */
  KeyModifiers KeyModifiersFromEvent(const QKeyEvent& event);
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Spire::KeyBindings::TaskBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Spire::KeyBindings::TaskBinding& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("node", value.m_node);
    }
  };

  template<>
  struct Shuttle<Spire::KeyBindings::CancelBinding> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Spire::KeyBindings::CancelBinding& value, unsigned int version) {
      shuttle.Shuttle("description", value.m_description);
      shuttle.Shuttle("type", value.m_type);
    }
  };
}
}

#endif
