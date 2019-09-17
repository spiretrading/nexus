#ifndef SPIRE_CANVASOBSERVER_HPP
#define SPIRE_CANVASOBSERVER_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Queues/TaskQueue.hpp>
#include <Beam/Reactors/Reactors.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/SignalHandling/ScopedSlotAdaptor.hpp>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2/signal.hpp>
#include <QTimer>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class CanvasObserver
      \brief Emits changes produced from a translated CanvasNode.
   */
  class CanvasObserver : private boost::noncopyable {
    public:

      //! Signals an update to a CanvasNode.
      /*!
        \param value The CanvasNode's updated value.
      */
      using UpdateSignal = boost::signals2::signal<
        void (const boost::any& value)>;

      //! Constructs a CanvasObserver.
      /*!
        \param target The CanvasNode to observe.
        \param observer The operation to perform on the <i>target</i>.
        \param context The <i>target</i>'s CanvasNodeTranslationContext.
        \param monitor The <i>target</i>'s ReactorMonitor.
        \param executingAccount The DirectoryEntry executing Orders.
        \param userProfile The user's profile.
      */
      CanvasObserver(const CanvasNode& target, const CanvasNode& observer,
        Beam::Ref<CanvasNodeTranslationContext> context,
        Beam::Ref<ReactorMonitor> monitor,
        const Beam::ServiceLocator::DirectoryEntry& executingAccount,
        Beam::Ref<UserProfile> userProfile);

      //! Returns the current value of the CanvasNode.
      const boost::any& GetValue() const;

      //! Connects a slot to the UpdateSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_type& slot) const;

    private:
      CanvasNodeTranslationContext* m_context;
      Beam::ServiceLocator::DirectoryEntry m_executingAccount;
      UserProfile* m_userProfile;
      std::vector<const CanvasNode*> m_dependencies;
      std::vector<const CanvasNode*> m_remainingDependencies;
      std::unique_ptr<CanvasNode> m_node;
      bool m_isTranslated;
      boost::any m_value;
      mutable UpdateSignal m_updateSignal;
      QTimer m_updateTimer;
      Beam::TaskQueue m_tasks;
      Beam::SignalHandling::ScopedSlotAdaptor m_callbacks;

      void Translate();
      void OnReactorUpdate(const boost::any& value);
      void OnUpdateTimer();
  };
}

#endif
