#include "Spire/Blotter/BlotterTasksModel.hpp"
#include <Beam/Queues/StateQueue.hpp>
#include <Beam/Tasks/ReactorMonitorTask.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <Beam/Utilities/Algorithm.hpp>
#include "Nexus/OrderExecutionService/StandardQueries.hpp"
#include "Nexus/OrderExecutionService/VirtualOrderExecutionClient.hpp"
#include "Nexus/Tasks/OrderWrapperTask.hpp"
#include "Spire/Blotter/BlotterModelUtilities.hpp"
#include "Spire/Blotter/BlotterTaskMonitor.hpp"
#include "Spire/Canvas/Common/BreadthFirstCanvasNodeIterator.hpp"
#include "Spire/Canvas/Common/CustomNode.hpp"
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeRefresh.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslationContext.hpp"
#include "Spire/Canvas/Operations/CanvasNodeTranslator.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/Operations/TranslationPreprocessor.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OrderWrapperTaskNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/SystemNodes/BlotterTaskMonitorNode.hpp"
#include "Spire/Canvas/SystemNodes/CanvasObserver.hpp"
#include "Spire/Canvas/TaskNodes/CanvasNodeTask.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/CustomQtVariants.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Reactors;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Tasks;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::Tasks;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
  const auto EXPIRY_INTERVAL = 500;

  void QueryDailyOrderSubmissions(const UserProfile& userProfile,
      const DirectoryEntry& account,
      const std::shared_ptr<QueueWriter<const Order*>>& queue) {
    Spawn(
      [&userProfile, account, queue] {
        auto currentTime = userProfile.GetServiceClients().GetTimeClient().
          GetTime();
        auto lastSequence = Beam::Queries::Sequence::First();
        for(auto& market : userProfile.GetMarketDatabase().GetEntries()) {
          auto timeOfDay = userProfile.GetServiceClients().GetTimeClient().
            GetTime();
          auto snapshotQuery = BuildDailyOrderSubmissionQuery(market.m_code,
            account, timeOfDay, timeOfDay, userProfile.GetMarketDatabase(),
            userProfile.GetTimeZoneDatabase());
          auto snapshotQueue = std::make_shared<Queue<SequencedOrder>>();
          userProfile.GetServiceClients().GetOrderExecutionClient().
            QueryOrderSubmissions(snapshotQuery, snapshotQueue);
          try {
            while(true) {
              queue->Push(snapshotQueue->Top().GetValue());
              lastSequence = std::max(lastSequence,
                snapshotQueue->Top().GetSequence());
              snapshotQueue->Pop();
            }
          } catch(const std::exception&) {}
        }
        AccountQuery query;
        query.SetIndex(account);
        query.SetSnapshotLimit(SnapshotLimit::Unlimited());
        if(lastSequence == Beam::Queries::Sequence::First()) {
          query.SetRange(currentTime, pos_infin);
        } else {
          query.SetRange(Increment(lastSequence), pos_infin);
        }
        userProfile.GetServiceClients().GetOrderExecutionClient().
          QueryOrderSubmissions(query, queue);
      });
  }
}

BlotterTasksModel::TaskContext::TaskContext(Ref<UserProfile> userProfile,
    const CanvasNode& node, const DirectoryEntry& executingAccount)
    : m_context(Ref(userProfile), Ref(m_reactorMonitor), executingAccount) {
  m_node = PreprocessTranslation(node);
  if(m_node == nullptr) {
    m_node = CanvasNode::Clone(node);
  }
  auto canvasNodeTaskFactory = CanvasNodeTaskFactory(Ref(m_context),
    Ref(*m_node));
  m_orderExecutionPublisher =
    canvasNodeTaskFactory.GetOrderExecutionPublisher();
  m_factory = ReactorMonitorTaskFactory(Ref(m_context.GetReactorMonitor()),
    canvasNodeTaskFactory);
  m_task = m_factory->Create();
}

BlotterTasksModel::ObserverEntry::ObserverEntry(string name,
    unique_ptr<CanvasObserver> observer)
    : m_name(std::move(name)),
      m_observer(std::move(observer)) {}

BlotterTasksModel::BlotterTasksModel(Ref<UserProfile> userProfile,
    const DirectoryEntry& executingAccount, bool isConsolidated,
    const BlotterTaskProperties& properties)
    : m_userProfile(userProfile.Get()),
      m_executingAccount(executingAccount),
      m_isConsolidated(isConsolidated),
      m_properties(properties),
      m_isRefreshing(false),
      m_properOrderExecutionPublisher(
        Initialize(UniqueFilter<const Order*>(), Initialize())) {
  m_taskSlotHandler.emplace();
  SetupLinkedOrderExecutionMonitor();
  if(m_isConsolidated) {
    m_userProfile->GetServiceClients().GetOrderExecutionClient().
      GetOrderSubmissionPublisher().Monitor(
      m_orderSlotHandler.GetSlot<const Order*>(
      std::bind(&BlotterTasksModel::OnOrderSubmitted, this,
      std::placeholders::_1)));
    auto orderQueue = std::make_shared<Queue<const Order*>>();
    QueryDailyOrderSubmissions(*m_userProfile, m_executingAccount, orderQueue);
    m_accountOrderPublisher =
      std::make_shared<QueuePublisher<SequencePublisher<const Order*>>>(
      orderQueue);
    m_accountOrderPublisher->Monitor(m_orderSlotHandler.GetSlot<const Order*>(
      std::bind(&BlotterTasksModel::OnOrderExecuted, this,
      std::placeholders::_1)));
  }
  connect(&m_updateTimer, &QTimer::timeout, this,
    &BlotterTasksModel::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  connect(&m_expiryTimer, &QTimer::timeout, this,
    &BlotterTasksModel::OnExpiryTimer);
  m_expiryTimer.start(EXPIRY_INTERVAL);
  for(const auto& monitor : m_properties.GetMonitors()) {
    m_taskMonitors.push_back(monitor);
  }
}

BlotterTasksModel::~BlotterTasksModel() {}

const BlotterTaskProperties& BlotterTasksModel::GetProperties() const {
  return m_properties;
}

void BlotterTasksModel::SetProperties(const BlotterTaskProperties& properties) {
  m_properties = properties;
  m_taskMonitors.clear();
  for(const auto& monitor : m_properties.GetMonitors()) {
    m_taskMonitors.push_back(monitor);
  }
  Refresh();
}

OrderExecutionPublisher& BlotterTasksModel::GetOrderExecutionPublisher() const {
  return const_cast<SpireAggregateOrderExecutionPublisher&>(
    *m_linkedOrderExecutionPublisher);
}

const BlotterTasksModel::TaskEntry& BlotterTasksModel::Add(
    const CanvasNode& node) {
  auto context = make_unique<TaskContext>(Ref(*m_userProfile), node,
    m_executingAccount);
  return Adopt(std::move(context));
}

const BlotterTasksModel::TaskEntry& BlotterTasksModel::Adopt(
    std::unique_ptr<TaskContext> context) {
  std::shared_ptr<TaskContext> adoptedContext = std::move(context);
  m_contexts.push_back(adoptedContext);
  m_properOrderExecutionPublisher.Add(
    *adoptedContext->m_orderExecutionPublisher);
  Monitor(adoptedContext);
  return *m_entries.back();
}

void BlotterTasksModel::Monitor(const std::shared_ptr<TaskContext>& context) {
  if(!m_taskIds.insert(context->m_task->GetId()).second) {
    return;
  }
  auto index = static_cast<int>(m_entries.size());
  beginInsertRows(QModelIndex(), index, index);
  auto entry = std::make_shared<TaskEntry>();
  entry->m_index = index;
  entry->m_sticky = false;
  entry->m_context = context;
  entry->m_context->m_task->GetPublisher().Monitor(
    m_taskSlotHandler->GetSlot<Task::StateEntry>(
    std::bind(&BlotterTasksModel::OnTaskState, this,
    std::weak_ptr<TaskEntry>(entry), std::placeholders::_1)));
  for(const auto& taskMonitor : m_taskMonitors) {
    auto observer = make_unique<CanvasObserver>(*entry->m_context->m_node,
      taskMonitor.GetMonitor(), Ref(context->m_context),
      Ref(context->m_reactorMonitor), m_executingAccount, Ref(*m_userProfile));
    auto monitor = make_unique<ObserverEntry>(taskMonitor.GetName(),
      std::move(observer));
    monitor->m_connection = monitor->m_observer->ConnectUpdateSignal(
      std::bind(&BlotterTasksModel::OnMonitorUpdate, this,
      std::weak_ptr<TaskEntry>(entry), taskMonitor.GetName(),
      std::placeholders::_1));
    entry->m_monitors.push_back(std::move(monitor));
  }
  m_entries.push_back(entry);
  endInsertRows();
  m_taskAddedSignal(*entry);
  for(const auto& link : m_outgoingLinks) {
    link->Monitor(context);
  }
}

const vector<std::shared_ptr<BlotterTasksModel::TaskContext>>&
    BlotterTasksModel::GetContexts() const {
  return m_contexts;
}

const vector<BlotterTasksModel*>& BlotterTasksModel::GetIncomingLinks() const {
  return m_incomingLinks;
}

void BlotterTasksModel::Refresh() {
  if(m_isRefreshing) {
    return;
  }
  beginResetModel();
  m_isRefreshing = true;
  if(!m_entries.empty()) {
    beginRemoveRows(QModelIndex(), 0, m_entries.size() - 1);
    vector<std::shared_ptr<TaskEntry>> entries;
    entries.swap(m_entries);
    m_pendingExpiryEntries.clear();
    m_expiredEntries.clear();
    m_taskSlotHandler = std::nullopt;
    m_taskSlotHandler.emplace();
    m_taskIds.clear();
    endRemoveRows();
    for(const auto& entry : entries) {
      m_taskRemovedSignal(*entry);
    }
  }
  for(const auto& link : m_outgoingLinks) {
    link->Refresh();
  }
  WithModels(*this,
    [&] (const BlotterTasksModel& model) {
      m_linkedOrderExecutionPublisher->Add(model.GetOrderExecutionPublisher());
      for(const auto& context : model.GetContexts()) {
        Monitor(context);
      }
    });
  m_isRefreshing = false;
  endResetModel();
}

void BlotterTasksModel::Link(Ref<BlotterTasksModel> model) {
  m_linkedOrderExecutionPublisher->Add(model->GetOrderExecutionPublisher());
  m_incomingLinks.push_back(model.Get());
  model->m_outgoingLinks.push_back(this);
  for(const auto& link : m_outgoingLinks) {
    link->Refresh();
  }
  WithModels(*model.Get(),
    [&] (const BlotterTasksModel& model) {
      for(const auto& context : model.GetContexts()) {
        Monitor(context);
      }
    });
}

void BlotterTasksModel::Unlink(BlotterTasksModel& model) {
  m_incomingLinks.erase(find(m_incomingLinks.begin(), m_incomingLinks.end(),
    &model));
  model.m_outgoingLinks.erase(find(model.m_outgoingLinks.begin(),
    model.m_outgoingLinks.end(), this));
  SetupLinkedOrderExecutionMonitor();
  Refresh();
}

const BlotterTasksModel::TaskEntry& BlotterTasksModel::GetEntry(
    int index) const {
  return *m_entries[index];
}

connection BlotterTasksModel::ConnectTaskAddedSignal(
    const TaskAddedSignal::slot_function_type& slot) const {
  return m_taskAddedSignal.connect(slot);
}

connection BlotterTasksModel::ConnectTaskRemovedSignal(
    const TaskRemovedSignal::slot_function_type& slot) const {
  return m_taskRemovedSignal.connect(slot);
}

int BlotterTasksModel::rowCount(const QModelIndex& parent) const {
  return static_cast<int>(m_entries.size());
}

int BlotterTasksModel::columnCount(const QModelIndex& parent) const {
  return STANDARD_COLUMN_COUNT + m_taskMonitors.size();
}

QVariant BlotterTasksModel::data(const QModelIndex& index, int role) const {
  if(!index.isValid()) {
    return QVariant();
  }
  auto& entry = *m_entries[index.row()];
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(index.column() == STICKY_COLUMN) {
      return entry.m_sticky;
    } else if(index.column() == NAME_COLUMN) {
      return QString::fromStdString(entry.m_context->m_node->GetText());
    } else if(index.column() == ID_COLUMN) {
      return entry.m_context->m_task->GetId();
    } else if(index.column() == STATE_COLUMN) {
      return QVariant::fromValue(entry.m_state);
    } else if(index.column() - STANDARD_COLUMN_COUNT >= 0 &&
        index.column() - STANDARD_COLUMN_COUNT < entry.m_monitors.size()) {
      auto value = entry.m_monitors[
        index.column() - STANDARD_COLUMN_COUNT]->m_observer->GetValue();
      return QVariant::fromValue(value);
    } else {
      return QVariant();
    }
  }
  return QVariant();
}

QVariant BlotterTasksModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if(role == Qt::TextAlignmentRole) {
    return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
  } else if(role == Qt::DisplayRole) {
    if(section == STICKY_COLUMN) {
      return QString("");
    } else if(section == NAME_COLUMN) {
      return QString("Name");
    } else if(section == ID_COLUMN) {
      return QString("ID");
    } else if(section == STATE_COLUMN) {
      return QString("State");
    } else if(section - STANDARD_COLUMN_COUNT >= 0 &&
        section - STANDARD_COLUMN_COUNT < m_taskMonitors.size()) {
      return QString::fromStdString(
        m_taskMonitors[section - STANDARD_COLUMN_COUNT].GetName());
    }
  }
  return QVariant();
}

bool BlotterTasksModel::setData(const QModelIndex& index,
    const QVariant& value, int role) {
  if(!index.isValid() || index.column() != STICKY_COLUMN ||
      role != Qt::EditRole || !value.canConvert<bool>()) {
    return false;
  }
  auto& entry = m_entries[index.row()];
  entry->m_sticky = value.value<bool>();
  if(!entry->m_sticky && IsTerminal(entry->m_state)) {
    m_pendingExpiryEntries.insert(entry);
  }
  Q_EMIT dataChanged(index, index);
  return true;
}

void BlotterTasksModel::SetupLinkedOrderExecutionMonitor() {
  m_linkedOrderExecutionPublisher.emplace(
    Initialize(UniqueFilter<const Order*>(), Initialize()));
  m_linkedOrderExecutionPublisher->Add(m_properOrderExecutionPublisher);
  if(m_isConsolidated && m_accountOrderPublisher != nullptr) {
    m_linkedOrderExecutionPublisher->Add(*m_accountOrderPublisher);
  }
}

void BlotterTasksModel::OnMonitorUpdate(std::weak_ptr<TaskEntry> weakEntry,
    const string& property, const any& value) {
  auto entry = weakEntry.lock();
  if(entry == nullptr) {
    return;
  }
  Q_EMIT dataChanged(index(entry->m_index, 0),
    index(entry->m_index, columnCount(QModelIndex()) - 1));
}

void BlotterTasksModel::OnTaskState(std::weak_ptr<TaskEntry> weakEntry,
    const Task::StateEntry& update) {
  auto entry = weakEntry.lock();
  if(entry == nullptr) {
    return;
  }
  entry->m_state = update.m_state;
  Q_EMIT dataChanged(index(entry->m_index, 0),
    index(entry->m_index, columnCount(QModelIndex()) - 1));
  if(!entry->m_sticky && IsTerminal(entry->m_state)) {
    m_pendingExpiryEntries.insert(entry);
  }
}

void BlotterTasksModel::OnOrderSubmitted(const Order* order) {
  m_submittedOrders.insert(order->GetInfo().m_orderId);
}

void BlotterTasksModel::OnOrderExecuted(const Order* order) {
  if(m_submittedOrders.find(order->GetInfo().m_orderId) !=
      m_submittedOrders.end()) {
    return;
  }
  m_properOrderExecutionPublisher.Push(order);
  auto stateQueue = std::make_shared<StateQueue<ExecutionReport>>();
  order->GetPublisher().Monitor(stateQueue);
  if(stateQueue->IsEmpty() || IsTerminal(stateQueue->Top().m_status)) {
    return;
  }
  auto node = make_unique<OrderWrapperTaskNode>(*order, *m_userProfile);
  auto context = make_unique<TaskContext>(Ref(*m_userProfile), *node,
    m_executingAccount);
  auto translation = Translate(context->m_context, *context->m_node);
  context->m_task->Execute();
  Adopt(std::move(context));
}

void BlotterTasksModel::OnUpdateTimer() {
  HandleTasks(m_orderSlotHandler);
  HandleTasks(*m_taskSlotHandler);
}

void BlotterTasksModel::OnExpiryTimer() {
  for(const auto& entry : m_expiredEntries) {
    int index = entry->m_index;
    if(index == -1) {
      continue;
    }
    entry->m_index = -1;
    beginRemoveRows(QModelIndex(), index, index);
    for(size_t j = index + 1; j < m_entries.size(); ++j) {
      assert(m_entries[j]->m_index > 0);
      --(m_entries[j]->m_index);
    }
    for(auto j = m_contexts.begin(); j != m_contexts.end(); ++j) {
      if(*j == entry->m_context) {
        Erase(m_contexts, j);
        break;
      }
    }
    m_entries.erase(m_entries.begin() + index);
    endRemoveRows();
    m_taskRemovedSignal(*entry);
  }
  m_expiredEntries.clear();
  m_expiredEntries.insert(m_expiredEntries.begin(),
    m_pendingExpiryEntries.begin(), m_pendingExpiryEntries.end());
  m_pendingExpiryEntries.clear();
}
