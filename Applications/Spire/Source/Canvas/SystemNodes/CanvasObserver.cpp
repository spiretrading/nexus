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
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/Canvas/ValueNodes/ValueNode.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

namespace {
  std::any get_initial_value(const CanvasNode& node) {
    if(dynamic_cast<const BlotterTaskMonitorNode*>(&node)) {
      return get_initial_value(node.GetChildren().front());
    }
    if(auto anchor = FindAnchor(node)) {
      if(auto literal = dynamic_cast<const BaseValueNode*>(&*anchor)) {
        return literal->to_any();
      }
    }
    return {};
  }
}

struct CanvasObserver::Evaluation {
  struct Translator {
    using type = ValueTypes;

    template<typename T>
    Box<void> operator ()(const Translation& translation,
        const std::shared_ptr<Evaluation>& evaluation) const {
      return box(lift([=] (const T& value) {
        evaluation->m_updates.push(value);
      }, translation.Extract<Box<T>>()));
    }
  };

  std::unique_ptr<CanvasNode> m_expression;
  std::unique_ptr<CanvasNodeTranslationContext> m_context;
  ScopedQueueWriter<std::any> m_updates;
  Shared<Cell<bool>> m_stop;
};

CanvasObserver::CanvasObserver(
    std::shared_ptr<Task> task, const CanvasNode& observer)
    : m_task(std::move(task)) {
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
  auto& monitor = *expression->FindChild("observer");
  m_value = get_initial_value(monitor);
  for(auto& child : BreadthFirstView(monitor)) {
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
  if(auto translation = PreprocessTranslation(*expression)) {
    expression = std::move(translation);
  }
  m_evaluation = std::make_shared<Evaluation>(std::move(expression),
    m_task->GetContext().make_isolated_context(), m_events.get_slot<std::any>(
      std::bind_front(&CanvasObserver::on_reactor_update, this)));
  QObject::connect(&m_translate_timer, &QTimer::timeout,
    std::bind_front(&CanvasObserver::translate, this));
  constexpr auto TRANSLATE_INTERVAL = 100;
  m_translate_timer.start(TRANSLATE_INTERVAL);
}

CanvasObserver::~CanvasObserver() {
  if(m_evaluation) {
    m_evaluation->m_stop->set_complete(true);
  }
}

const CanvasObserver::Type& CanvasObserver::get() const {
  return m_value;
}

connection CanvasObserver::connect_update_signal(
    const UpdateSignal::slot_type& slot) const {
  return m_update_signal.connect(slot);
}

void CanvasObserver::translate() {
  try {
    while(!m_remaining_dependencies.empty()) {
      auto dependency = m_remaining_dependencies.back();
      if(!m_task->GetContext().FindSubTranslation(*dependency)) {
        return;
      }
      m_remaining_dependencies.pop_back();
    }
    for(auto& dependency : m_dependencies) {
      auto& mirror =
        *m_evaluation->m_expression->GetChildren().front().FindNode(
          GetFullName(*dependency));
      Mirror(*dependency, m_task->GetContext(), mirror,
        out(*m_evaluation->m_context));
    }
    auto observer = Translate(*m_evaluation->m_context,
      m_evaluation->m_expression->GetChildren().back());
    auto reactor = instantiate<Evaluation::Translator>(observer.GetTypeInfo())(
      observer, m_evaluation);
    m_task->GetExecutor().Add(
      box(until(m_evaluation->m_stop, std::move(reactor))));
    m_translate_timer.stop();
  } catch(const std::exception&) {
    return;
  }
}

void CanvasObserver::on_reactor_update(const std::any& value) {
  m_value = value;
  m_update_signal(m_value);
}
