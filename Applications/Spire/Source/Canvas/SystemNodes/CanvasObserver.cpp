#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include <Beam/Reactors/FunctionReactor.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Types/TaskType.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace std;

namespace {
  const unsigned int UPDATE_INTERVAL = 100;

  struct ObserverTranslator {
    template<typename T>
    static std::shared_ptr<BaseReactor> Template(
        const std::shared_ptr<BaseReactor>& observer,
        const std::function<void (const any& value)>& callback) {
      return MakeFunctionReactor(
        [=] (const T& value) {
          callback(value);
          return value;
        }, std::static_pointer_cast<Reactor<T>>(observer));
    }

    typedef ValueTypes SupportedTypes;
  };
}

CanvasObserver::CanvasObserver(const CanvasNode& target,
    const CanvasNode& observer, RefType<CanvasNodeTranslationContext> context,
    RefType<ReactorMonitor> monitor, const DirectoryEntry& executingAccount,
    RefType<UserProfile> userProfile)
    : m_context(context.Get()),
      m_isTranslated(false),
      m_executingAccount(executingAccount),
      m_userProfile(userProfile.Get()) {
  vector<CustomNode::Child> children;
  CustomNode::Child targetChild("target", target.GetType());
  children.push_back(targetChild);
  CustomNode::Child observerChild("observer", observer.GetType());
  children.push_back(observerChild);
  unique_ptr<CanvasNode> customObserver = make_unique<CustomNode>("Observer",
    children);
  customObserver = customObserver->Replace("target", CanvasNode::Clone(target));
  customObserver = customObserver->Replace("observer",
    CanvasNode::Clone(observer));
  customObserver = Refresh(std::move(customObserver));
  auto validationErrors = Validate(*customObserver);
  if(!validationErrors.empty()) {
    return;
  }
  auto& taskNode = customObserver->GetChildren().front();
  for(const auto& child :
      BreadthFirstView(*customObserver->FindChild("observer"))) {
    if(auto reference = dynamic_cast<const ReferenceNode*>(&child)) {
      auto& referent = *reference->FindReferent();
      if(IsParent(taskNode, referent) || &referent == &taskNode) {
        auto path = GetPath(taskNode, referent);
        auto& dependency = *target.FindNode(path);
        m_dependencies.push_back(&dependency);
        m_remainingDependencies.push_back(&dependency);
      }
    }
  }
  m_node = std::move(customObserver);
  unique_ptr<CanvasNode> translatedNode = PreprocessTranslation(*m_node);
  if(translatedNode != nullptr) {
    m_node = std::move(translatedNode);
  }
  QObject::connect(&m_updateTimer, &QTimer::timeout,
    std::bind(&CanvasObserver::OnUpdateTimer, this));
  m_updateTimer.start(UPDATE_INTERVAL);
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
    bool monitorTranslated = false;
    while(!m_remainingDependencies.empty()) {
      auto dependency = m_remainingDependencies.back();
      if(m_context->FindSubTranslation(*dependency).is_initialized()) {
        m_remainingDependencies.pop_back();
      } else {
        break;
      }
    }
    if(m_remainingDependencies.empty()) {
      CanvasNodeTranslationContext context(Ref(*m_userProfile),
        Ref(m_context->GetReactorMonitor()), m_executingAccount);
      for(const auto& rootDependency : m_dependencies) {
        auto fullName = GetFullName(*rootDependency);
        auto monitorDependency =
          &*m_node->GetChildren().front().FindNode(fullName);
        Mirror(*rootDependency, *m_context, *monitorDependency, Store(context));
      }
      auto observerReactor = boost::get<std::shared_ptr<BaseReactor>>(
        Spire::Translate(context, m_node->GetChildren().back()));
      auto reactor = Instantiate<ObserverTranslator>(
        observerReactor->GetType())(observerReactor, m_callbacks.GetCallback(
        std::bind(&CanvasObserver::OnReactorUpdate, this,
        std::placeholders::_1)));
      m_context->GetReactorMonitor().Add(reactor);
      monitorTranslated = true;
      m_isTranslated = true;
    }
  } catch(const std::exception&) {
    return;
  }
}

void CanvasObserver::OnReactorUpdate(const any& value) {
  m_tasks.Push(
    [=] {
      m_value = value;
      m_updateSignal(m_value);
    });
}

void CanvasObserver::OnUpdateTimer() {
  Translate();
  HandleTasks(m_tasks);
}
