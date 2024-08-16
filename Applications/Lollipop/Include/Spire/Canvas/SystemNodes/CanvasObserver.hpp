#ifndef SPIRE_CANVAS_OBSERVER_HPP
#define SPIRE_CANVAS_OBSERVER_HPP
#include <memory>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Reactors/Reactors.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/SignalHandling/ScopedSlotAdaptor.hpp>
#include <boost/any.hpp>
#include <boost/signals2/signal.hpp>
#include <QTimer>
#include "Spire/Async/EventHandler.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Emits changes produced from a translated CanvasNode. */
  class CanvasObserver {
    public:

      /**
       * Signals an update to a CanvasNode.
       * @param value The CanvasNode's updated value.
       */
      using UpdateSignal =
        boost::signals2::signal<void (const boost::any& value)>;

      /**
       * Constructs a CanvasObserver.
       * @param task The task the <i>target</i> belongs to.
       * @param observer The operation to perform on the <i>target</i>.
       */
      CanvasObserver(std::shared_ptr<Task> task, const CanvasNode& observer);

      /** Returns the current value of the CanvasNode. */
      const boost::any& GetValue() const;

      /**
       * Connects a slot to the UpdateSignal.
       * @param slot The slot to connect to the signal.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_type& slot) const;

    private:
      std::shared_ptr<Task> m_task;
      std::unique_ptr<CanvasNode> m_observer;
      std::vector<const CanvasNode*> m_dependencies;
      std::vector<const CanvasNode*> m_remainingDependencies;
      bool m_isTranslated;
      boost::any m_value;
      mutable UpdateSignal m_updateSignal;
      QTimer m_translateTimer;
      Beam::SignalHandling::ScopedSlotAdaptor m_callbacks;
      EventHandler m_eventHandler;

      CanvasObserver(const CanvasObserver&) = delete;
      CanvasObserver& operator =(const CanvasObserver&) = delete;
      void Translate();
      void OnReactorUpdate(const boost::any& value);
  };
}

#endif
