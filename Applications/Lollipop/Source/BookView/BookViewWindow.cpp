#include "Spire/BookView/BookViewWindow.hpp"
#include <tuple>
#include <Beam/Utilities/Algorithm.hpp>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableView>
#include "Spire/Blotter/BlotterModel.hpp"
#include "Spire/Blotter/BlotterSettings.hpp"
#include "Spire/Blotter/BlotterWindow.hpp"
#include "Spire/BookView/BookViewPanel.hpp"
#include "Spire/BookView/BookViewPropertiesDialog.hpp"
#include "Spire/BookView/BookViewWindowSettings.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValidator.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/SingleOrderTaskNode.hpp"
#include "Spire/Canvas/SystemNodes/InteractionsNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/CanvasView/CondensedCanvasWidget.hpp"
#include "Spire/CanvasView/CanvasNodeNotVisibleException.hpp"
#include "Spire/InputWidgets/TickerInputDialog.hpp"
#include "Spire/UI/LinkTickerContextAction.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "Spire/UI/ValueLabel.hpp"
#include "Spire/Utilities/TickerTechnicalsModel.hpp"
#include "ui_BookViewWindow.h"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

namespace {
  const auto UPDATE_INTERVAL = 100;

  BookViewWindow* FindNextWindow(const BookViewWindow& origin) {
    auto widgets = QApplication::topLevelWidgets();
    auto rightClosestPosition = std::numeric_limits<int>::max();
    auto rightClosestWindow = static_cast<BookViewWindow*>(nullptr);
    auto bottomLeftPosition =
      QPoint(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    auto bottomLeftWindow = static_cast<BookViewWindow*>(nullptr);
    auto topLeftPosition =
      QPoint(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    auto topLeftWindow = static_cast<BookViewWindow*>(nullptr);
    for(auto& widget : widgets) {
      auto candidate = dynamic_cast<BookViewWindow*>(widget);
      if(widget != &origin && candidate) {
        auto position = widget->pos();
        if(position.y() >= origin.pos().y() - 20 &&
            position.y() <= origin.pos().y() + 20 &&
            position.x() >= origin.pos().x() &&
            position.x() <= rightClosestPosition) {
          rightClosestWindow = candidate;
          rightClosestPosition = rightClosestWindow->pos().x();
        }
        if(position.y() > origin.pos().y() &&
            position.x() <= bottomLeftPosition.x() &&
            position.y() <= bottomLeftPosition.y()) {
          bottomLeftWindow = candidate;
          bottomLeftPosition = bottomLeftWindow->pos();
        }
        if(std::make_tuple(position.y(), position.x()) <=
            std::make_tuple(topLeftPosition.y(), topLeftPosition.x())) {
          topLeftWindow = candidate;
          topLeftPosition = topLeftWindow->pos();
        }
      }
    }
    if(rightClosestWindow) {
      return rightClosestWindow;
    }
    if(bottomLeftWindow) {
      return bottomLeftWindow;
    }
    if(topLeftWindow) {
      return topLeftWindow;
    }
    return nullptr;
  }

  BookViewWindow* FindPreviousWindow(const BookViewWindow& origin) {
    auto previous = static_cast<BookViewWindow*>(nullptr);
    auto next = FindNextWindow(origin);
    while(next != nullptr && next != &origin) {
      previous = next;
      next = FindNextWindow(*next);
    }
    return previous;
  }
}

BookViewWindow::BookViewWindow(Ref<UserProfile> userProfile,
    const BookViewProperties& properties, const std::string& identifier,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame(parent, flags),
      TickerContext(identifier),
      m_ui(std::make_unique<Ui_BookViewWindow>()),
      m_userProfile(userProfile.get()),
      m_properties(properties),
      m_taskEntryWidget(nullptr),
      m_isTaskEntryWidgetForInteractionsProperties(false),
      m_bidPanelGuard(false),
      m_askPanelGuard(false) {
  m_ui->setupUi(this);
  m_ui->m_highValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_lowValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_defaultQuantityValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_volumeValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_openValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_closeValue->Initialize("", Ref(*m_userProfile));
  m_ui->m_bidPanel->Initialize(Ref(*m_userProfile), m_properties, Side::BID);
  m_ui->m_askPanel->Initialize(Ref(*m_userProfile), m_properties, Side::ASK);
  m_ui->m_askPanel->GetQuoteList().setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ui->m_askPanel->GetQuoteList(),
    &QTableView::customContextMenuRequested, this,
    &BookViewWindow::OnContextMenu);
  m_ui->m_bidPanel->GetQuoteList().setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ui->m_bidPanel->GetQuoteList(),
    &QTableView::customContextMenuRequested, this,
    &BookViewWindow::OnContextMenu);
  connect(
    &m_updateTimer, &QTimer::timeout, this, &BookViewWindow::OnUpdateTimer);
  m_updateTimer.start(UPDATE_INTERVAL);
  SetupTickerTechnicalsModel();
}

const BookViewProperties& BookViewWindow::GetBookViewProperties() const {
  return m_properties;
}

void BookViewWindow::SetProperties(const BookViewProperties& properties) {
  m_properties = properties;
  m_ui->m_askPanel->SetProperties(m_properties);
  m_ui->m_bidPanel->SetProperties(m_properties);
}

void BookViewWindow::DisplayTicker(const Ticker& ticker) {
  m_ticker = ticker;
  setWindowTitle(displayText(ticker) + tr(" - Book View"));
  m_ui->m_askPanel->DisplayTicker(m_ticker);
  m_ui->m_bidPanel->DisplayTicker(m_ticker);
  SetupTickerTechnicalsModel();
  SetDisplayedTicker(m_ticker);
}

std::unique_ptr<WindowSettings> BookViewWindow::GetWindowSettings() const {
  return std::make_unique<BookViewWindowSettings>(*this, Ref(*m_userProfile));
}

void BookViewWindow::showEvent(QShowEvent* event) {
  if(auto context = TickerContext::FindTickerContext(m_linkIdentifier)) {
    Link(*context);
  } else {
    m_linkConnection.disconnect();
    m_linkIdentifier.clear();
  }
  QFrame::showEvent(event);
  m_ui->m_askPanel->GetQuoteList().horizontalHeader()->setStretchLastSection(
    true);
  m_ui->m_bidPanel->GetQuoteList().horizontalHeader()->setStretchLastSection(
    true);
}

void BookViewWindow::closeEvent(QCloseEvent* event) {
  if(m_ticker != Ticker()) {
    auto settings =
      std::make_unique<BookViewWindowSettings>(*this, Ref(*m_userProfile));
    m_userProfile->AddRecentlyClosedWindow(std::move(settings));
  }
  QFrame::closeEvent(event);
}

void BookViewWindow::keyPressEvent(QKeyEvent* event) {
  if(m_taskEntryWidget) {
    HandleTaskInputEvent(event);
  } else if(event->key() == Qt::Key_Tab) {
    if(auto window = FindNextWindow(*this)) {
      window->activateWindow();
    }
    return;
  } else if(event->key() == Qt::Key_Backtab) {
    if(auto window = FindPreviousWindow(*this)) {
      window->activateWindow();
    }
    return;
  } else if(m_ticker != Ticker()) {
    auto key =
      QKeySequence(static_cast<int>(event->modifiers() + event->key()));
    if(key.matches(Qt::Key_QuoteLeft) == QKeySequence::ExactMatch) {
      HandleInteractionsPropertiesEvent();
    } else {
      auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
        m_ticker.get_venue(), key);
      if(taskBinding) {
        HandleKeyBindingEvent(*taskBinding);
        return;
      }
      auto cancelBinding =
        m_userProfile->GetKeyBindings().GetCancelFromBinding(key);
      if(cancelBinding) {
        HandleCancelBindingEvent(*cancelBinding);
        return;
      }
    }
  }
  HandleTickerInputEvent(event);
}

void BookViewWindow::HandleLink(TickerContext& context) {
  m_linkIdentifier = context.GetIdentifier();
  m_linkConnection = context.ConnectTickerDisplaySignal(
    std::bind_front(&BookViewWindow::DisplayTicker, this));
  DisplayTicker(context.GetDisplayedTicker());
}

void BookViewWindow::HandleUnlink() {
  m_linkConnection.disconnect();
  m_linkIdentifier.clear();
}

void BookViewWindow::SetupTickerTechnicalsModel() {
  m_tickerTechnicalsConnections.disconnect();
  m_ui->m_openValue->Reset();
  m_ui->m_closeValue->Reset();
  m_ui->m_highValue->Reset();
  m_ui->m_lowValue->Reset();
  m_ui->m_volumeValue->Reset();
  m_tickerTechnicalsModel =
    TickerTechnicalsModel::GetModel(Ref(*m_userProfile), m_ticker);
  m_tickerTechnicalsConnections.add(
    m_tickerTechnicalsModel->ConnectOpenSignal(
      std::bind_front(&BookViewWindow::OnOpenUpdate, this)));
  m_tickerTechnicalsConnections.add(
    m_tickerTechnicalsModel->ConnectCloseSignal(
      std::bind_front(&BookViewWindow::OnCloseUpdate, this)));
  m_tickerTechnicalsConnections.add(
    m_tickerTechnicalsModel->ConnectHighSignal(
      std::bind_front(&BookViewWindow::OnHighUpdate, this)));
  m_tickerTechnicalsConnections.add(
    m_tickerTechnicalsModel->ConnectLowSignal(
      std::bind_front(&BookViewWindow::OnLowUpdate, this)));
  m_tickerTechnicalsConnections.add(
    m_tickerTechnicalsModel->ConnectVolumeSignal(
      std::bind_front(&BookViewWindow::OnVolumeUpdate, this)));
}

std::unique_ptr<CanvasNode>
    BookViewWindow::PrepareTaskNode(const CanvasNode& node) {
  auto taskNode = CanvasNode::Clone(node);
  auto tickerNode =
    taskNode->FindNode(SingleOrderTaskNode::SECURITY_PROPERTY);
  if(tickerNode && !tickerNode->IsReadOnly()) {
    if(auto tickerValueNode =
        dynamic_cast<const TickerNode*>(&*tickerNode)) {
      auto builder = CanvasNodeBuilder(*taskNode);
      builder.Replace(*tickerNode, tickerValueNode->SetValue(m_ticker,
        m_userProfile->GetVenueDatabase()));
      builder.SetReadOnly(*tickerNode, true);
      auto price = [&] {
        if(auto sideNode =
            taskNode->FindNode(SingleOrderTaskNode::SIDE_PROPERTY)) {
          if(auto sideValueNode = dynamic_cast<const SideNode*>(&*sideNode)) {
            if(sideValueNode->GetValue() == Side::BID) {
              return m_ui->m_bidPanel->GetBestQuote().m_price;
            }
            return m_ui->m_askPanel->GetBestQuote().m_price;
          }
          return m_ui->m_bidPanel->GetBestQuote().m_price;
        }
        return m_ui->m_bidPanel->GetBestQuote().m_price;
      }();
      auto priceNode = taskNode->FindNode(SingleOrderTaskNode::PRICE_PROPERTY);
      if(priceNode && !priceNode->IsReadOnly()) {
        if(auto moneyNode = dynamic_cast<const MoneyNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetValue(price));
        } else if(auto moneyNode =
            dynamic_cast<const OptionalPriceNode*>(&*priceNode)) {
          builder.Replace(*priceNode, moneyNode->SetReferencePrice(price));
        }
      }
      auto quantityNode =
        taskNode->FindNode(SingleOrderTaskNode::QUANTITY_PROPERTY);
      if(quantityNode && !quantityNode->IsReadOnly()) {
        if(auto quantityValueNode =
            dynamic_cast<const IntegerNode*>(&*quantityNode)) {
          auto sideNode =
            taskNode->FindNode(SingleOrderTaskNode::SIDE_PROPERTY);
          auto quantity = [&] {
            if(sideNode) {
              if(auto sideValueNode =
                  dynamic_cast<const SideNode*>(&*sideNode)) {
                return m_userProfile->GetDefaultQuantity(
                  m_ticker, sideValueNode->GetValue());
              } else {
                return m_userProfile->GetInteractionProperties().get(
                  m_ticker).m_defaultQuantity;
              }
            } else {
              return m_userProfile->GetInteractionProperties().get(
                m_ticker).m_defaultQuantity;
            }
          }();
          builder.Replace(*quantityNode,
            quantityValueNode->SetValue(static_cast<int>(quantity)));
        }
      }
      taskNode = builder.Make();
    }
  }
  return taskNode;
}

void BookViewWindow::RemoveTaskEntry() {
  setUpdatesEnabled(false);
  m_ui->verticalLayout->removeWidget(m_taskEntryWidget);
  m_taskEntryWidget->deleteLater();
  m_taskEntryWidget = nullptr;
  m_isTaskEntryWidgetForInteractionsProperties = false;
  setUpdatesEnabled(true);
}

void BookViewWindow::ExecuteTask(const CanvasNode& node) {
  auto errors = Validate(node);
  if(!errors.empty()) {
    QMessageBox::warning(this, QObject::tr("Error"),
      QString::fromStdString(errors.front().GetErrorMessage()));
    return;
  }
  auto& activeBlotter = m_userProfile->GetBlotterSettings().GetActiveBlotter();
  auto& blotterWindow =
    BlotterWindow::GetBlotterWindow(Ref(*m_userProfile), Ref(activeBlotter));
  if(!blotterWindow.isVisible()) {
    blotterWindow.setAttribute(Qt::WA_ShowWithoutActivating);
    blotterWindow.show();
    raise();
  }
  auto& entry = activeBlotter.GetTasksModel().Add(node);
  m_tasksExecuted[m_ticker].push_back(entry.m_task);
  entry.m_task->GetPublisher().monitor(
    m_eventHandler.get_slot<Task::StateEntry>(
      [=, task = entry.m_task] (const auto& update) {
        OnTaskState(task, update);
      }));
  entry.m_task->Execute();
}

void BookViewWindow::HandleTickerInputEvent(QKeyEvent* event) {
  auto key = event->key();
  if(key == Qt::Key_PageUp) {
    m_tickerViewStack.PushUp(m_ticker, [&] (const auto& ticker) {
      DisplayTicker(ticker);
    });
    return;
  } else if(key == Qt::Key_PageDown) {
    m_tickerViewStack.PushDown(m_ticker, [&] (const auto& ticker) {
      DisplayTicker(ticker);
    });
    return;
  }
  auto text = event->text();
  if(text.isEmpty() || !text[0].isLetterOrNumber()) {
    return;
  }
  ShowTickerInputDialog(Ref(*m_userProfile), text.toStdString(), this,
    [=] (auto ticker) {
      if(!ticker || ticker == Ticker() || ticker == m_ticker) {
        return;
      }
      m_tickerViewStack.Push(m_ticker);
      DisplayTicker(*ticker);
    });
}

void BookViewWindow::HandleKeyBindingEvent(
    const KeyBindings::TaskBinding& keyBinding) {
  auto taskNode = PrepareTaskNode(*keyBinding.m_node);
  m_taskEntryWidget =
    new CondensedCanvasWidget(keyBinding.m_name, Ref(*m_userProfile), this);
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  auto isVisible = [&] {
    try {
      m_taskEntryWidget->Add(coordinate, *taskNode);
      return true;
    } catch(const CanvasNodeNotVisibleException&) {
      return false;
    }
  }();
  if(isVisible) {
    m_taskEntryWidget->setSizePolicy(
      QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_ui->verticalLayout->insertWidget(2, m_taskEntryWidget);
    m_taskEntryWidget->Focus();
  } else {
    m_taskEntryWidget->deleteLater();
    m_taskEntryWidget = nullptr;
    ExecuteTask(*taskNode);
  }
}

void BookViewWindow::HandleInteractionsPropertiesEvent() {
  auto& interactionsProperties =
    m_userProfile->GetInteractionProperties().get(m_ticker);
  auto interactionsNode = std::make_unique<InteractionsNode>(
    m_ticker, m_userProfile->GetVenueDatabase(), interactionsProperties);
  m_taskEntryWidget =
    new CondensedCanvasWidget("Interactions", Ref(*m_userProfile), this);
  m_isTaskEntryWidgetForInteractionsProperties = true;
  auto coordinate = CanvasNodeModel::Coordinate(0, 0);
  m_taskEntryWidget->Add(coordinate, *interactionsNode);
  m_taskEntryWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_ui->verticalLayout->insertWidget(2, m_taskEntryWidget);
  m_taskEntryWidget->Focus();
}

void BookViewWindow::HandleCancelBindingEvent(
    const KeyBindings::CancelBinding& cancelBinding) {
  KeyBindings::CancelBinding::HandleCancel(
    cancelBinding, out(m_tasksExecuted[m_ticker]));
}

void BookViewWindow::HandleTaskInputEvent(QKeyEvent* event) {
  auto baseKey = event->key();
  if(baseKey == Qt::Key_Escape) {
    RemoveTaskEntry();
  } else if(baseKey == Qt::Key_Enter || baseKey == Qt::Key_Return) {
    if(m_isTaskEntryWidgetForInteractionsProperties) {
      auto& interactionsNode = static_cast<const InteractionsNode&>(
        *m_taskEntryWidget->GetRoots().front());
      m_userProfile->GetInteractionProperties().set(
        m_ticker, interactionsNode.GetProperties());
      RemoveTaskEntry();
    } else {
      auto taskNode = CanvasNode::Clone(*m_taskEntryWidget->GetRoots().front());
      RemoveTaskEntry();
      ExecuteTask(*taskNode);
    }
  } else if(baseKey == Qt::Key_QuoteLeft) {
    RemoveTaskEntry();
    HandleInteractionsPropertiesEvent();
  } else {
    auto key =
      QKeySequence(static_cast<int>(event->modifiers() + event->key()));
    auto taskBinding = m_userProfile->GetKeyBindings().GetTaskFromBinding(
      m_ticker.get_venue(), key);
    if(taskBinding) {
      RemoveTaskEntry();
      HandleKeyBindingEvent(*taskBinding);
    }
  }
}

void BookViewWindow::UpdateDefaultQuantity() {
  auto defaultBidQuantity =
    m_userProfile->GetDefaultQuantity(m_ticker, Side::BID);
  auto defaultAskQuantity =
    m_userProfile->GetDefaultQuantity(m_ticker, Side::ASK);
  m_ui->m_defaultQuantityValue->SetValue(QString::fromStdString(
    lexical_cast<std::string>(defaultBidQuantity) + "x" +
    lexical_cast<std::string>(defaultAskQuantity)));
}

void BookViewWindow::OnOpenUpdate(Money open) {
  m_ui->m_openValue->SetValue(QVariant::fromValue(open));
}

void BookViewWindow::OnCloseUpdate(Money close) {
  m_ui->m_closeValue->SetValue(QVariant::fromValue(close));
}

void BookViewWindow::OnHighUpdate(Money high) {
  m_ui->m_highValue->SetValue(QVariant::fromValue(high));
}

void BookViewWindow::OnLowUpdate(Money low) {
  m_ui->m_lowValue->SetValue(QVariant::fromValue(low));
}

void BookViewWindow::OnVolumeUpdate(Quantity volume) {
  m_ui->m_volumeValue->SetValue(static_cast<int>(volume));
}

void BookViewWindow::OnTaskState(const std::shared_ptr<Task>& task,
    const Task::StateEntry& update) {
  if(IsTerminal(update.m_state)) {
    remove_first(m_tasksExecuted[m_ticker], task);
  }
}

void BookViewWindow::OnContextMenu(const QPoint& position) {
  auto contextMenu = QMenu();
  auto propertiesAction = QAction(&contextMenu);
  propertiesAction.setText(tr("Properties"));
  propertiesAction.setToolTip(tr("Opens the Book View properties."));
  contextMenu.addAction(&propertiesAction);
  auto linkMenu = QMenu("Links");
  auto linkActions = LinkTickerContextAction::MakeActions(
    this, m_linkIdentifier, &linkMenu, *m_userProfile);
  for(auto& action : linkActions) {
    linkMenu.addAction(action.get());
  }
  if(!linkMenu.isEmpty()) {
    contextMenu.addMenu(&linkMenu);
  }
  auto selectedAction =
    contextMenu.exec(static_cast<QWidget*>(sender())->mapToGlobal(position));
  if(selectedAction == &propertiesAction) {
    auto propertiesWidget = BookViewPropertiesDialog(
      Ref(*m_userProfile), m_ticker, m_properties, this);
    if(propertiesWidget.exec() == QDialog::Rejected) {
      return;
    }
    SetProperties(propertiesWidget.GetProperties());
  } else if(auto linkAction =
      dynamic_cast<LinkTickerContextAction*>(selectedAction)) {
    linkAction->Execute(out(*this));
  }
}

void BookViewWindow::OnUpdateTimer() {
  UpdateDefaultQuantity();
}
