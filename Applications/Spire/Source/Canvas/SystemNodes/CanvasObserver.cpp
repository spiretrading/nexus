#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include <functional>
#include <Beam/Utilities/Instantiate.hpp>
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/Translation.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

namespace {
  struct ObserverTranslator {
    using type = ValueTypes;

    template<typename T>
    Aspen::Box<void> operator ()(const Translation& translation,
        const std::function<void (const std::any& value)>& callback) const {
      return Aspen::box(Aspen::lift([=] (const T& value) {
        callback(value);
      }, translation.Extract<Aspen::Box<T>>()));
    }
  };
}

CanvasObserver::CanvasObserver(
    std::shared_ptr<Task> task, const CanvasNode& observer)
    : m_task(std::move(task)),
      m_is_translated(false) {
  auto children = std::vector<CustomNode::Child>();
  children.emplace_back("target", m_task->GetNode().GetType());
  children.emplace_back("observer", observer.GetType());
  auto expression = Refresh(
    std::make_unique<CustomNode>("Observer", std::move(children))->Replace(
      "target", CanvasNode::Clone(m_task->GetNode()))->Replace(
      "observer", CanvasNode::Clone(observer)));
  if(!Validate(*expression).empty()) {
    return;
  }
  auto& target = expression->GetChildren().front();
  for(auto& child : BreadthFirstView(*expression->FindChild("observer"))) {
    if(auto reference = dynamic_cast<const ReferenceNode*>(&child)) {
      auto& referent = *reference->FindReferent();
      if(IsParent(target, referent) || &referent == &target) {
        auto path = GetPath(target, referent);
        auto& dependency = *m_task->GetNode().FindNode(path);
        m_dependencies.push_back(&dependency);
      }
    }
  }
  m_remaining_dependencies = m_dependencies;
  m_observer = std::move(expression);
  if(auto translation = PreprocessTranslation(*m_observer)) {
    m_observer = std::move(translation);
  }
  QObject::connect(&m_translate_timer, &QTimer::timeout,
    std::bind_front(&CanvasObserver::translate, this));
  constexpr auto TRANSLATE_INTERVAL = 100;
  m_translate_timer.start(TRANSLATE_INTERVAL);
}

CanvasObserver::~CanvasObserver() = default;

const std::any& CanvasObserver::get_value() const {
  return m_value;
}

connection CanvasObserver::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void CanvasObserver::translate() {
  if(m_is_translated) {
    return;
  }
  try {
    while(!m_remaining_dependencies.empty()) {
      auto dependency = m_remaining_dependencies.back();
      if(!m_task->GetContext().FindSubTranslation(*dependency)) {
        return;
      }
      m_remaining_dependencies.pop_back();
    }
    for(auto& dependency : m_dependencies) {
      auto& mirror = *m_observer->GetChildren().front().FindNode(
        GetFullName(*dependency));
      Mirror(
        *dependency, m_task->GetContext(), mirror, out(m_task->GetContext()));
    }
    auto observer =
      Translate(m_task->GetContext(), m_observer->GetChildren().back());
    auto reactor = instantiate<ObserverTranslator>(
      observer.GetTypeInfo())(observer, m_callbacks.make_slot(
        std::bind_front(&CanvasObserver::on_reactor_update, this)));
    m_task->GetExecutor().Add(std::move(reactor));
    m_is_translated = true;
  } catch(const std::exception&) {
    return;
  }
}

void CanvasObserver::on_reactor_update(const std::any& value) {
  m_event_handler.push([=, this] {
    m_value = value;
    m_update_signal(m_value);
  });
}
