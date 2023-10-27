#include "Spire/Canvas/Tasks/Task.hpp"
#include <Beam/Reactors/PublisherReactor.hpp>
#include "Nexus/OrderExecutionService/OrderCancellationReactor.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

namespace {
  auto nextId = 0;
}

Task::StateEntry::StateEntry(State state)
  : m_state(state) {}

Task::StateEntry::StateEntry(State state, const std::string& message)
  : m_state(state),
    m_message(message) {}

Task::Task(const CanvasNode& node, DirectoryEntry executingAccount,
  Ref<UserProfile> userProfile)
  : m_node(
      [&] {
        auto n = PreprocessTranslation(node);
        if(n == nullptr) {
          return CanvasNode::Clone(node);
        }
        return n;
      }()),
    m_context(Ref(userProfile), Ref(m_executor), std::move(executingAccount)),
    m_isExecutable(true),
    m_isFailed(false),
    m_isCancelable(true),
    m_id(++nextId),
    m_state(State::READY) {}

const CanvasNode& Task::GetNode() const {
  return *m_node;
}

CanvasNodeTranslationContext& Task::GetContext() {
  return m_context;
}

const CanvasNodeTranslationContext& Task::GetContext() const {
  return m_context;
}

Executor& Task::GetExecutor() {
  return m_executor;
}

const Executor& Task::GetExecutor() const {
  return m_executor;
}

int Task::GetId() const {
  return m_id;
}

void Task::Execute() {
  if(!m_isExecutable) {
    return;
  }
  m_isExecutable = false;
  auto translation = Translate(m_context, *m_node).Extract<Aspen::Box<void>>();
  m_executor.Add(Aspen::box(
    Aspen::chain(
      Aspen::lift([=] {
        m_state = State::INITIALIZING;
        m_publisher.Push(StateEntry(m_state));
      }),
      Aspen::until(m_cancelToken,
        Aspen::lift([=] (const Aspen::Maybe<void>& value) {
          if(m_state == State::INITIALIZING) {
            m_state = State::ACTIVE;
            m_publisher.Push(StateEntry(m_state));
          }
          try {
            value.get();
          } catch(...) {
            m_isFailed = true;
            m_cancelToken->set_complete(true);
          }
        }, std::move(translation))),
      Aspen::lift([=] {
        if(!m_isCancelable) {
          m_state = State::PENDING_CANCEL;
          m_publisher.Push(StateEntry(m_state));
        }
        m_context.GetOrderPublisher().Break();
      }),
      Aspen::until(!m_cancelToken,
        Aspen::Box<void>(OrderCancellationReactor(
          Ref(m_context.GetUserProfile().GetServiceClients().
          GetOrderExecutionClient()), PublisherReactor(
          m_context.GetOrderPublisher())))),
      Aspen::lift([=] {
        if(m_isFailed) {
          m_state = State::FAILED;
        } else if(m_state == State::PENDING_CANCEL) {
          m_state = State::CANCELED;
        } else {
          m_state = State::COMPLETE;
        }
        m_publisher.Push(StateEntry(m_state));
        m_executor.Close();
      }))));
  m_executor.Open();
}

void Task::Cancel() {
  if(m_isExecutable) {
    m_isExecutable = false;
    m_publisher.Push(StateEntry(State::PENDING_CANCEL));
    m_state = State::CANCELED;
    m_publisher.Push(StateEntry(m_state, "Canceled by user."));
  } else if(m_isCancelable.exchange(false)) {
    m_cancelToken->set_complete(true);
  }
}

const Publisher<Task::StateEntry>& Task::GetPublisher() const {
  return m_publisher;
}

std::ostream& Spire::operator <<(std::ostream& s, Task::State state) {
  if(state == Task::State::READY) {
    s << "Ready";
  } else if(state == Task::State::INITIALIZING) {
    s << "Initializing";
  } else if(state == Task::State::ACTIVE) {
    s << "Active";
  } else if(state == Task::State::PENDING_CANCEL) {
    s << "Pending Cancel";
  } else if(state == Task::State::CANCELED) {
    s << "Canceled";
  } else if(state == Task::State::FAILED) {
    s << "Failed";
  } else if(state == Task::State::COMPLETE) {
    s << "Complete";
  }
  return s;
}

bool Spire::IsTerminal(Task::State state) {
  return state == Task::State::CANCELED || state == Task::State::FAILED ||
    state == Task::State::COMPLETE;
}
