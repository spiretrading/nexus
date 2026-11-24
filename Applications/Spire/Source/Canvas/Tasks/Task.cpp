#include "Spire/Canvas/Tasks/Task.hpp"
#include <atomic>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/PublisherReactor.hpp>
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Nexus;
using namespace Spire;

namespace {
  std::atomic_int nextId = 0;
}

struct Task::Execution {
  std::unique_ptr<CanvasNode> m_node;
  Spire::Executor m_executor;
  CanvasNodeTranslationContext m_context;
  bool m_isExecutable;
  bool m_isFailed;
  std::atomic_bool m_isCancelable;
  State m_state;
  Shared<Cell<bool>> m_cancelToken;
  int m_id;
  SequencePublisher<StateEntry> m_publisher;

  Execution(const CanvasNode& node, DirectoryEntry account,
    Ref<UserProfile> profile);

  void execute(std::shared_ptr<Execution> lifetime);
  void cancel();
};

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, const std::string& message)
  : m_state(state),
    m_message(message) {}

Task::Task(const CanvasNode& node, DirectoryEntry executingAccount,
  Ref<UserProfile> userProfile)
  : m_execution(std::make_shared<Execution>(
      node, std::move(executingAccount), userProfile)) {}

Task::~Task() {
  m_execution->m_executor.Close();
}

const CanvasNode& Task::GetNode() const {
  return *m_execution->m_node;
}

CanvasNodeTranslationContext& Task::GetContext() {
  return m_execution->m_context;
}

const CanvasNodeTranslationContext& Task::GetContext() const {
  return m_execution->m_context;
}

Spire::Executor& Task::GetExecutor() {
  return m_execution->m_executor;
}

const Spire::Executor& Task::GetExecutor() const {
  return m_execution->m_executor;
}

int Task::GetId() const {
  return m_execution->m_id;
}

void Task::Execute() {
  m_execution->execute(m_execution);
}

void Task::Cancel() {
  m_execution->cancel();
}

const Publisher<Task::StateEntry>& Task::GetPublisher() const {
  return m_execution->m_publisher;
}

Task::Execution::Execution(const CanvasNode& node, DirectoryEntry account,
  Ref<UserProfile> profile)
  : m_node(
      [&] {
        auto n = PreprocessTranslation(node);
        if(!n) {
          return CanvasNode::Clone(node);
        }
        return n;
      }()),
    m_context(profile, Ref(m_executor), std::move(account)),
    m_isExecutable(true),
    m_isFailed(false),
    m_isCancelable(true),
    m_state(State::READY),
    m_id(++nextId) {}

void Task::Execution::execute(std::shared_ptr<Execution> lifetime) {
  if(!m_isExecutable) {
    return;
  }
  m_isExecutable = false;
  auto translation = Translate(m_context, *m_node).Extract<Aspen::Box<void>>();
  m_executor.Add(box(
    chain(
      lift([=, this] {
        m_state = State::INITIALIZING;
        m_publisher.push(StateEntry(m_state));
      }),
      until(m_cancelToken,
        lift([=, this] (const Maybe<void>& value) {
          if(m_state == State::INITIALIZING) {
            m_state = State::ACTIVE;
            m_publisher.push(StateEntry(m_state));
          }
          try {
            value.get();
          } catch(...) {
            m_isFailed = true;
            m_cancelToken->set_complete(true);
          }
        }, std::move(translation))),
      lift([=, this] {
        if(!m_isCancelable) {
          m_state = State::PENDING_CANCEL;
          m_publisher.push(StateEntry(m_state));
        }
        m_context.GetOrderPublisher().close();
      }),
      until(!m_cancelToken,
        Aspen::Box<void>(OrderCancellationReactor(Ref(
          m_context.GetUserProfile().GetClients().get_order_execution_client()),
          publisher_reactor(m_context.GetOrderPublisher())))),
      lift([=, this] {
        if(m_isFailed) {
          m_state = State::FAILED;
        } else if(m_state == State::PENDING_CANCEL) {
          m_state = State::CANCELED;
        } else {
          m_state = State::COMPLETE;
        }
        m_publisher.push(StateEntry(m_state));
      }))));
  m_executor.Open(std::move(lifetime));
}

void Task::Execution::cancel() {
  if(m_isExecutable) {
    m_isExecutable = false;
    m_publisher.push(StateEntry(State::PENDING_CANCEL));
    m_state = State::CANCELED;
    m_publisher.push(StateEntry(m_state, "Canceled by user."));
  } else if(m_isCancelable.exchange(false)) {
    m_cancelToken->set_complete(true);
  }
}

std::ostream& Spire::operator <<(std::ostream& out, Task::State state) {
  return out << std::format(out.getloc(), "{}", state);
}

bool Spire::IsTerminal(Task::State state) {
  return state == Task::State::CANCELED || state == Task::State::FAILED ||
    state == Task::State::COMPLETE;
}

std::format_context::iterator std::formatter<Task::State>::format(
    Task::State state, std::format_context& context) const {
  auto text = [&] {
    if(state == Task::State::READY) {
      return "Ready";
    } else if(state == Task::State::INITIALIZING) {
      return "Initializing";
    } else if(state == Task::State::ACTIVE) {
      return "Active";
    } else if(state == Task::State::PENDING_CANCEL) {
      return "Pending Cancel";
    } else if(state == Task::State::CANCELED) {
      return "Canceled";
    } else if(state == Task::State::FAILED) {
      return "Failed";
    } else if(state == Task::State::COMPLETE) {
      return "Complete";
    }
    return "";
  }();
  return std::formatter<std::string_view>::format(text, context);
}
