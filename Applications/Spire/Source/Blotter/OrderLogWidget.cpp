#include "Spire/Blotter/OrderLogWidget.hpp"
#include <QKeyEvent>
#include "Spire/Blotter/OrderLogModel.hpp"
#include "Spire/Blotter/OrderLogPropertiesDialog.hpp"
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/LegacyUI/CustomQtVariants.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_OrderLogWidget.h"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  class OrderLogFilterProxyModel : public CustomVariantSortFilterProxyModel {
    public:
      OrderLogFilterProxyModel(Ref<UserProfile> userProfile,
          Ref<OrderLogModel> sourceModel)
          : CustomVariantSortFilterProxyModel(Ref(userProfile)),
            m_sourceModel(sourceModel.Get()) {
        setSourceModel(m_sourceModel);
      }

    protected:
      bool filterAcceptsRow(
          int sourceRow, const QModelIndex& sourceParent) const override {
        auto index = m_sourceModel->index(sourceRow, 0);
        if(!index.isValid()) {
          return CustomVariantSortFilterProxyModel::filterAcceptsRow(
            sourceRow, sourceParent);
        }
        auto& entry = m_sourceModel->GetEntry(index);
        auto& properties = m_sourceModel->GetProperties();
        if(properties.m_orderStatusFilterType ==
            OrderLogProperties::OrderStatusFilterType::LIVE_ORDERS &&
            IsTerminal(entry.m_status) || properties.m_orderStatusFilterType ==
            OrderLogProperties::OrderStatusFilterType::TERMINAL_ORDERS &&
            !IsTerminal(entry.m_status) || properties.m_orderStatusFilterType ==
            OrderLogProperties::OrderStatusFilterType::CUSTOM &&
            !properties.m_orderStatusFilter.Test(entry.m_status)) {
          return false;
        }
        return CustomVariantSortFilterProxyModel::filterAcceptsRow(
          sourceRow, sourceParent);
      }

    private:
      OrderLogModel* m_sourceModel;
  };
}

OrderLogWidget::OrderLogWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_OrderLogWidget>()),
      m_model(nullptr) {
  m_ui->setupUi(this);
  m_ui->m_orderLogTable->installEventFilter(this);
  connect(m_ui->m_orderLogTable, &QTableView::customContextMenuRequested, this,
    &OrderLogWidget::OnContextMenu);
}

OrderLogWidget::UIState OrderLogWidget::GetUIState() const {
  auto state = UIState();
  state.m_tableGeometry =
    m_ui->m_orderLogTable->horizontalHeader()->saveGeometry();
  state.m_tableState = m_ui->m_orderLogTable->horizontalHeader()->saveState();
  return state;
}

void OrderLogWidget::SetUIState(const UIState& state) {
  restore_geometry(
    *m_ui->m_orderLogTable->horizontalHeader(), state.m_tableGeometry);
  m_ui->m_orderLogTable->horizontalHeader()->restoreState(state.m_tableState);
}

void OrderLogWidget::SetModel(
    Ref<UserProfile> userProfile, Ref<BlotterModel> model) {
  m_userProfile = userProfile.Get();
  m_orderEntries.clear();
  m_ui->m_orderLogTable->reset();
  m_model = model.Get();
  m_proxyModel = std::make_unique<OrderLogFilterProxyModel>(
    Ref(userProfile), Ref(m_model->GetOrderLogModel()));
  m_ui->m_orderLogTable->setModel(m_proxyModel.get());
  m_ui->m_orderLogTable->setItemDelegate(
    new CustomVariantItemDelegate(Ref(userProfile)));
  m_ui->m_orderLogTable->horizontalHeader()->setSectionsMovable(true);
  auto metrics = QFontMetrics(m_ui->m_orderLogTable->font());
  m_ui->m_orderLogTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 4);
  for(auto i = 0;
      i < m_model->GetOrderLogModel().rowCount(QModelIndex()); ++i) {
    auto index = m_model->GetOrderLogModel().index(i, 0);
    OnOrderAdded(m_model->GetOrderLogModel().GetEntry(index));
  }
  m_orderAddedConnection = m_model->GetOrderLogModel().ConnectOrderAddedSignal(
    std::bind_front(&OrderLogWidget::OnOrderAdded, this));
  m_orderRemovedConnection =
    m_model->GetOrderLogModel().ConnectOrderRemovedSignal(
      std::bind_front(&OrderLogWidget::OnOrderRemoved, this));
  connect(m_proxyModel.get(), &OrderLogFilterProxyModel::rowsInserted, this,
    &OrderLogWidget::OnProxyOrderAdded);
  connect(m_proxyModel.get(), &OrderLogFilterProxyModel::rowsAboutToBeRemoved,
    this, &OrderLogWidget::OnProxyOrderRemoved);
}

bool OrderLogWidget::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_orderLogTable) {
    if(auto keyEvent = dynamic_cast<QKeyEvent*>(event)) {
      if(!m_ui->m_orderLogTable->selectionModel()->selectedRows().empty() &&
          keyEvent->modifiers() == 0 && keyEvent->key() == Qt::Key_Escape) {
        OnCancel();
        return true;
      }
      auto key = QKeySequence(keyEvent->modifiers() + keyEvent->key());
      auto cancelBinding = m_userProfile->
        GetKeyBindings()->get_cancel_key_bindings()->find_operation(key);
      if(cancelBinding) {
        execute(*cancelBinding,
          m_userProfile->GetServiceClients().GetOrderExecutionClient(),
          Store(m_orderEntries));
        return true;
      }
    }
  }
  return QWidget::eventFilter(object, event);
}

void OrderLogWidget::OnOrderAdded(const OrderLogModel::OrderEntry& entry) {
  m_orderEntries.push_back(entry);
  entry.m_order->GetPublisher().Monitor(
    m_eventHandler.get_slot<ExecutionReport>(
      std::bind_front(
        &OrderLogWidget::OnExecutionReport, this, entry.m_order)));
}

void OrderLogWidget::OnOrderRemoved(const OrderLogModel::OrderEntry& entry) {
  auto orderIterator = std::find_if(m_orderEntries.begin(),
    m_orderEntries.end(), [&] (const auto& e) {
      return entry.m_order == e.m_order;
    });
  if(orderIterator == m_orderEntries.end()) {
    return;
  }
  m_orderEntries.erase(orderIterator);
}

void OrderLogWidget::OnProxyOrderAdded(
    const QModelIndex& parent, int first, int last) {
  for(auto i = first; i <= last; ++i) {
    auto& entry = m_model->GetOrderLogModel().GetEntry(
      m_proxyModel->mapToSource(m_proxyModel->index(i, 0)));
    auto orderData = JsonObject();
    orderData["blotter_id"] = reinterpret_cast<std::intptr_t>(parentWidget());
    orderData["order_id"] =
      static_cast<double>(entry.m_order->GetInfo().m_orderId);
  }
}

void OrderLogWidget::OnProxyOrderRemoved(
    const QModelIndex& parent, int first, int last) {
  for(auto i = first; i <= last; ++i) {
    auto& entry = m_model->GetOrderLogModel().GetEntry(
      m_proxyModel->mapToSource(m_proxyModel->index(i, 0)));
    auto orderData = JsonObject();
    orderData["blotter_id"] = reinterpret_cast<std::intptr_t>(parentWidget());
    orderData["order_id"] =
      static_cast<double>(entry.m_order->GetInfo().m_orderId);
  }
}

void OrderLogWidget::OnCancel() {
  auto indexes = m_ui->m_orderLogTable->selectionModel()->selectedRows();
  for(auto& index : indexes) {
    auto& entry =
      m_model->GetOrderLogModel().GetEntry(m_proxyModel->mapToSource(index));
    m_userProfile->GetServiceClients().GetOrderExecutionClient().Cancel(
      *entry.m_order);
  }
}

void OrderLogWidget::OnExecutionReport(
    const Order* order, const ExecutionReport& executionReport) {
  auto orderIterator = std::find_if(m_orderEntries.begin(),
    m_orderEntries.end(), [&] (const auto& entry) {
      return entry.m_order == order;
    });
  if(orderIterator == m_orderEntries.end()) {
    return;
  }
  orderIterator->m_status = executionReport.m_status;
  if(!IsTerminal(executionReport.m_status)) {
    return;
  }
  m_orderEntries.erase(orderIterator);
}

void OrderLogWidget::OnContextMenu(const QPoint& point) {
  auto dialog =
    OrderLogPropertiesDialog(Ref(*m_userProfile), Ref(*m_model), this);
  dialog.exec();
}
