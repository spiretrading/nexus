#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include <Aspen/Aspen.hpp>
#include <Beam/Utilities/Instantiate.hpp>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  constexpr auto TRANSLATE_INTERVAL = 100;

  struct ObserverTranslator {
    using type = ValueTypes;

    template<typename T>
    Aspen::Box<void> operator ()(const Translation& translation,
        const std::function<void (const any& value)>& callback) const {
      return Aspen::box(Aspen::lift([=] (const T& value) {
        callback(value);
      }, translation.Extract<Aspen::Box<T>>()));
    }
  };
}

CanvasObserver::CanvasObserver(
    std::shared_ptr<Task> task, const CanvasNode& observer)
    : m_task(std::move(task)),
      m_isTranslated(false) {
  auto children = std::vector<CustomNode::Child>();
  children.push_back(CustomNode::Child("target", m_task->GetNode().GetType()));
  children.push_back(CustomNode::Child("observer", observer.GetType()));
  auto customObserver = Refresh(
    std::make_unique<CustomNode>("Observer", std::move(children))->Replace(
      "target", CanvasNode::Clone(m_task->GetNode()))->Replace(
      "observer", CanvasNode::Clone(observer)));
  auto validationErrors = Validate(*customObserver);
  if(!validationErrors.empty()) {
    return;
  }
  auto& taskNode = customObserver->GetChildren().front();
  for(auto& child : BreadthFirstView(*customObserver->FindChild("observer"))) {
    if(auto reference = dynamic_cast<const ReferenceNode*>(&child)) {
      auto& referent = *reference->FindReferent();
      if(IsParent(taskNode, referent) || &referent == &taskNode) {
        auto path = GetPath(taskNode, referent);
        auto& dependency = *m_task->GetNode().FindNode(path);
        m_dependencies.push_back(&dependency);
        m_remainingDependencies.push_back(&dependency);
      }
    }
  }
  m_observer = std::move(customObserver);
  if(auto translatedNode = PreprocessTranslation(*m_observer)) {
    m_observer = std::move(translatedNode);
  }
  QObject::connect(&m_translateTimer, &QTimer::timeout,
    std::bind_front(&CanvasObserver::Translate, this));
  m_translateTimer.start(TRANSLATE_INTERVAL);
}

const any& CanvasObserver::GetValue() const {
  return m_value;
}

connection CanvasObserver::ConnectUpdateSignal(
    const UpdateSignal::slot_type& slot) const {
  return m_updateSignal.connect(slot);
}

void CanvasObserver::Translate() {
  if(m_isTranslated) {
    return;
  }
  try {
    while(!m_remainingDependencies.empty()) {
      auto dependency = m_remainingDependencies.back();
      if(m_task->GetContext().FindSubTranslation(*dependency)) {
        m_remainingDependencies.pop_back();
      } else {
        break;
      }
    }
    if(m_remainingDependencies.empty()) {
      for(auto& rootDependency : m_dependencies) {
        auto monitorDependency = &*m_observer->GetChildren().front().FindNode(
          GetFullName(*rootDependency));
        Mirror(*rootDependency, m_task->GetContext(), *monitorDependency,
          out(m_task->GetContext()));
      }
      auto observer = Spire::Translate(
        m_task->GetContext(), m_observer->GetChildren().back());
      auto reactor = instantiate<ObserverTranslator>(
        observer.GetTypeInfo())(observer, m_callbacks.make_slot(
          std::bind_front(&CanvasObserver::OnReactorUpdate, this)));
      m_task->GetExecutor().Add(std::move(reactor));
      m_isTranslated = true;
    }
  } catch(const std::exception&) {
    return;
  }
}

void CanvasObserver::OnReactorUpdate(const any& value) {
  m_eventHandler.push([=] {
    m_value = value;
    m_updateSignal(m_value);
  });
}
