#ifndef SPIRE_ORDERTASKVIEW_HPP
#define SPIRE_ORDERTASKVIEW_HPP
#include <unordered_map>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <boost/noncopyable.hpp>
#include <QKeyEvent>
#include "Spire/Blotter/BlotterTasksModel.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasView.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/UI/UI.hpp"

namespace Spire {

  /*! \class OrderTaskView
      \brief Stores the necessary state to display an CondensedCanvasWidget for
             an Order CanvasNode within a widget.
   */
  class OrderTaskView : private boost::noncopyable {
    public:

      //! Specifies the type of slot used to display a CondensedCanvasWidget.
      /*!
        \param widget The CondensedCanvasWidget to display.
      */
      using DisplayWidgetSlot = std::function<
        void (CondensedCanvasWidget& widget)>;

      //! Specifies the type of slot used to remove a CondensedCanvasWidget.
      /*!
        \param widget The CondensedCanvasWidget to remove.
      */
      using RemoveWidgetSlot = std::function<
        void (CondensedCanvasWidget& widget)>;

      //! Constructs an OrderTaskView.
      /*!
        \param displayWidget The slot used to display a CondensedCanvasWidget.
        \param removeWidgetSlot The slot used to remove a CondensedCanvasWidget.
        \param parent The parent widget.
        \param param userProfile The user's profile.
      */
      OrderTaskView(const DisplayWidgetSlot& displayWidgetSlot,
        const RemoveWidgetSlot& removeWidgetSlot,
        Beam::Ref<QWidget> parent, Beam::Ref<UserProfile> userProfile);

      //! Handles a key event sent to the parent widget.
      /*!
        \param event The event sent to the parent widget to handle.
        \param security The Security represented by the widget.
        \param askPrice The price to use as an ask.
        \param bidPrice The price to use as a bid.
        \return <code>true</code> iff the event was handled by this view.
      */
      bool HandleKeyPressEvent(const QKeyEvent& event,
        const Nexus::Security& security, Nexus::Money askPrice,
        Nexus::Money bidPrice);

    private:
      struct State {
        const Nexus::Security* m_security;
        const Nexus::Money* m_askPrice;
        const Nexus::Money* m_bidPrice;
      };
      QWidget* m_parent;
      UserProfile* m_userProfile;
      bool m_isTaskEntryWidgetForInteractionsProperties;
      std::unordered_map<Nexus::Security, std::vector<std::shared_ptr<Task>>>
        m_tasksExecuted;
      State* m_state;
      CondensedCanvasWidget* m_taskEntryWidget;
      DisplayWidgetSlot m_displayWidgetSlot;
      RemoveWidgetSlot m_removeWidgetSlot;
      Beam::TaskQueue m_slotHandler;

      void RemoveTaskEntry();
      void ExecuteTask(const CanvasNode& node);
      std::unique_ptr<CanvasNode> InitializeTaskNode(
        const CanvasNode& baseNode);
      bool HandleKeyBindingEvent(const KeyBindings::TaskBinding& keyBinding);
      bool HandleTaskInputEvent(const QKeyEvent& event);
      void HandleInteractionsPropertiesEvent();
      bool HandleCancelBindingEvent(
        const KeyBindings::CancelBinding& keyBinding);
      void OnTaskState(const std::shared_ptr<Task>& task,
        const Nexus::Security& security, const Task::StateEntry& update);
  };
}

#endif
