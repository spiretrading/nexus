#include "Spire/CanvasView/OpenCanvasEditor.hpp"
#include <limits>
#include <boost/lexical_cast.hpp>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include "Spire/Canvas/Common/CanvasNodeOperations.hpp"
#include "Spire/Canvas/Common/CanvasNodeVisitor.hpp"
#include "Spire/Canvas/IONodes/FilePathNode.hpp"
#include "Spire/Canvas/IONodes/FileReaderNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Operations/CanvasNodeValueEquality.hpp"
#include "Spire/Canvas/OrderExecutionNodes/MaxFloorNode.hpp"
#include "Spire/Canvas/OrderExecutionNodes/OptionalPriceNode.hpp"
#include "Spire/Canvas/Records/QueryNode.hpp"
#include "Spire/Canvas/ReferenceNodes/ReferenceNode.hpp"
#include "Spire/Canvas/StandardNodes/TimeRangeParameterNode.hpp"
#include "Spire/Canvas/ValueNodes/BooleanNode.hpp"
#include "Spire/Canvas/ValueNodes/CurrencyNode.hpp"
#include "Spire/Canvas/ValueNodes/DateTimeNode.hpp"
#include "Spire/Canvas/ValueNodes/DecimalNode.hpp"
#include "Spire/Canvas/ValueNodes/DestinationNode.hpp"
#include "Spire/Canvas/ValueNodes/DurationNode.hpp"
#include "Spire/Canvas/ValueNodes/IntegerNode.hpp"
#include "Spire/Canvas/ValueNodes/MarketNode.hpp"
#include "Spire/Canvas/ValueNodes/MoneyNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderStatusNode.hpp"
#include "Spire/Canvas/ValueNodes/OrderTypeNode.hpp"
#include "Spire/Canvas/ValueNodes/SecurityNode.hpp"
#include "Spire/Canvas/ValueNodes/SideNode.hpp"
#include "Spire/Canvas/ValueNodes/TextNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeInForceNode.hpp"
#include "Spire/Canvas/ValueNodes/TimeNode.hpp"
#include "Spire/CanvasView/CustomNodeDialog.hpp"
#include "Spire/CanvasView/FileReaderNodeDialog.hpp"
#include "Spire/CanvasView/LuaScriptDialog.hpp"
#include "Spire/CanvasView/ReplaceNodeCommand.hpp"
#include "Spire/Catalog/CatalogEntry.hpp"
#include "Spire/Catalog/CatalogWindow.hpp"
#include "Spire/InputWidgets/DateTimeInputDialog.hpp"
#include "Spire/InputWidgets/SecurityInputDialog.hpp"
#include "Spire/InputWidgets/TimeInputDialog.hpp"
#include "Spire/InputWidgets/TimeRangeInputDialog.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/MaxFloorSpinBox.hpp"
#include "Spire/UI/MoneySpinBox.hpp"
#include "Spire/UI/OptionalPriceSpinBox.hpp"
#include "Spire/UI/QuantitySpinBox.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;

namespace {
  class OpenEditorCanvasNodeVisitor : public CanvasNodeVisitor {
    public:
      OpenEditorCanvasNodeVisitor(Ref<const CanvasNode> node,
        Ref<CanvasNodeModel> model, Ref<UserProfile> userProfile,
        QEvent* event);
      CanvasNodeEditor::EditVariant GetEditor();
      void Visit(const BooleanNode& node) override;
      void Visit(const CurrencyNode& node) override;
      void Visit(const CustomNode& node) override;
      void Visit(const DateTimeNode& node) override;
      void Visit(const DecimalNode& node) override;
      void Visit(const DestinationNode& node) override;
      void Visit(const DurationNode& node) override;
      void Visit(const FilePathNode& node) override;
      void Visit(const FileReaderNode& node) override;
      void Visit(const IntegerNode& node) override;
      void Visit(const LuaScriptNode& node) override;
      void Visit(const MarketNode& node) override;
      void Visit(const MaxFloorNode& node) override;
      void Visit(const MoneyNode& node) override;
      void Visit(const OptionalPriceNode& node) override;
      void Visit(const OrderStatusNode& node) override;
      void Visit(const OrderTypeNode& node) override;
      void Visit(const QueryNode& node) override;
      void Visit(const ReferenceNode& node) override;
      void Visit(const SecurityNode& node) override;
      void Visit(const SideNode& node) override;
      void Visit(const TextNode& node) override;
      void Visit(const TimeInForceNode& node) override;
      void Visit(const TimeNode& node) override;
      void Visit(const TimeRangeParameterNode& node) override;
      void Visit(const CanvasNode& node) override;

    private:
      const CanvasNode* m_node;
      CanvasNodeModel* m_model;
      UserProfile* m_userProfile;
      QEvent* m_event;
      CanvasNodeEditor::EditVariant m_editVariant;
  };
}

CanvasNodeEditor::EditVariant Spire::OpenCanvasEditor(const CanvasNode& node,
    CanvasNodeModel& model, UserProfile& userProfile, QEvent* event) {
  auto visitor =
    OpenEditorCanvasNodeVisitor(Ref(node), Ref(model), Ref(userProfile), event);
  return visitor.GetEditor();
}

OpenEditorCanvasNodeVisitor::OpenEditorCanvasNodeVisitor(
  Ref<const CanvasNode> node, Ref<CanvasNodeModel> model,
  Ref<UserProfile> userProfile, QEvent* event)
  : m_node(node.Get()),
    m_model(model.Get()),
    m_userProfile(userProfile.Get()),
    m_event(event) {}

CanvasNodeEditor::EditVariant OpenEditorCanvasNodeVisitor::GetEditor() {
  m_editVariant = static_cast<QUndoCommand*>(nullptr);
  m_node->Apply(*this);
  return m_editVariant;
}

void OpenEditorCanvasNodeVisitor::Visit(const BooleanNode& node) {
  auto editor = new QComboBox();
  editor->addItem("true");
  editor->addItem("false");
  if(node.GetValue()) {
    editor->setCurrentIndex(0);
  } else {
    editor->setCurrentIndex(1);
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const CurrencyNode& node) {
  auto editor = new QComboBox();
  auto& currencies = m_userProfile->GetCurrencyDatabase().GetEntries();
  for(auto i = std::size_t(0); i != currencies.size(); ++i) {
    auto& entry = currencies[i];
    editor->addItem(QString::fromStdString(entry.m_code.GetData()));
    if(node.GetValue() != CurrencyId::NONE && entry.m_id == node.GetValue()) {
      editor->setCurrentIndex(i);
    }
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const CustomNode& node) {
  auto dialog = CustomNodeDialog(
    node, Ref(*m_userProfile), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *dialog.GetNode());
}

void OpenEditorCanvasNodeVisitor::Visit(const DateTimeNode& node) {
  auto dialog = DateTimeInputDialog(
    node.GetValue(), Ref(*m_userProfile), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto previousValue = node.GetValue();
  auto newValue = dialog.GetDateTime();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *node.SetValue(newValue));
}

void OpenEditorCanvasNodeVisitor::Visit(const DecimalNode& node) {
  auto editor = new QDoubleSpinBox();
  editor->setMaximum(10000000);
  editor->setMinimum(-10000000);
  if(m_event && m_event->type() == QEvent::KeyPress) {
    auto text = static_cast<QKeyEvent*>(m_event)->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->setValue(node.GetValue());
    } else {
      editor->clear();
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->setValue(node.GetValue());
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const DestinationNode& node) {
  auto editor = new QComboBox();
  auto marketCode = node.GetMarket();
  auto& destinationDatabase = m_userProfile->GetDestinationDatabase();
  auto destinations = destinationDatabase.SelectEntries(
    [=] (const auto& entry) {
      if(marketCode.IsEmpty()) {
        return true;
      }
      return find(entry.m_markets.begin(), entry.m_markets.end(), marketCode) !=
        entry.m_markets.end();
    });
  if(m_userProfile->IsAdministrator() &&
      destinationDatabase.GetManualOrderEntryDestination().is_initialized()) {
    destinations.push_back(
      *destinationDatabase.GetManualOrderEntryDestination());
  }
  for(auto i = destinations.begin(); i != destinations.end(); ++i) {
    editor->addItem(QString::fromStdString(i->m_id));
    if(i->m_id == node.GetValue()) {
      editor->setCurrentIndex(std::distance(destinations.begin(), i));
    }
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const DurationNode& node) {
  auto editor = new QTimeEdit();
  editor->setDisplayFormat("hh:mm:ss.zzz");
  auto timeDisplay = QTime(0, 0, 0, 0);
  timeDisplay = timeDisplay.addMSecs(
    static_cast<int>(node.GetValue().total_milliseconds()));
  if(m_event && m_event->type() == QEvent::KeyPress) {
    auto text = static_cast<QKeyEvent*>(m_event)->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->setTime(timeDisplay);
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->setTime(timeDisplay);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const FilePathNode& node) {
  auto previousValue = node.GetPath();
  auto newValue = QFileDialog::getOpenFileName(dynamic_cast<QWidget*>(m_model),
    "Select File", QString::fromStdString(node.GetPath())).toStdString();
  if(newValue.empty() || newValue == previousValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *node.SetPath(newValue));
}

void OpenEditorCanvasNodeVisitor::Visit(const FileReaderNode& node) {
  auto dialog = FileReaderNodeDialog(
    node, Ref(*m_userProfile), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *dialog.GetNode());
}

void OpenEditorCanvasNodeVisitor::Visit(const IntegerNode& node) {
  auto editor = new QuantitySpinBox(Ref(*m_userProfile), node);
  if(m_event && m_event->type() == QEvent::KeyPress) {
    auto text = static_cast<QKeyEvent*>(m_event)->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->setValue(static_cast<int>(node.GetValue()));
    } else {
      editor->clear();
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->setValue(static_cast<int>(node.GetValue()));
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const LuaScriptNode& node) {
  auto dialog =
    LuaScriptDialog(node, Ref(*m_userProfile), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *dialog.GetNode());
}

void OpenEditorCanvasNodeVisitor::Visit(const MarketNode& node) {
  auto editor = new QComboBox();
  auto& markets = m_userProfile->GetMarketDatabase().GetEntries();
  for(auto i = std::size_t(0); i != markets.size(); ++i) {
    auto& entry = markets[i];
    editor->addItem(QString::fromStdString(entry.m_code.GetData()));
    if(!node.GetValue().IsEmpty() && entry.m_code == node.GetValue()) {
      editor->setCurrentIndex(i);
    }
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const MaxFloorNode& node) {
  auto keyEvent = dynamic_cast<QKeyEvent*>(m_event);
  if(keyEvent) {
    if(keyEvent->modifiers() == 0 && keyEvent->key() == Qt::Key_Delete) {
      if(node.GetValue() == -1) {
        return;
      }
      auto coordinate = m_model->GetCoordinate(node);
      m_editVariant =
        new ReplaceNodeCommand(Ref(*m_model), coordinate, *node.SetValue(-1));
      return;
    }
  }
  auto editor = new MaxFloorSpinBox(Ref(*m_userProfile), node);
  if(keyEvent) {
    auto text = keyEvent->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->setValue(static_cast<int>(node.GetValue()));
    } else {
      editor->clear();
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->setValue(static_cast<int>(node.GetValue()));
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const MoneyNode& node) {
  auto editor = new MoneySpinBox(Ref(*m_userProfile), node);
  if(m_event && m_event->type() == QEvent::KeyPress) {
    auto text = static_cast<QKeyEvent*>(m_event)->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->SetValue(node.GetValue());
    } else {
      editor->clear();
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->SetValue(node.GetValue());
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const OptionalPriceNode& node) {
  auto keyEvent = dynamic_cast<QKeyEvent*>(m_event);
  if(keyEvent) {
    if(keyEvent->modifiers() == 0 && keyEvent->key() == Qt::Key_Delete) {
      if(node.GetValue() == Money::ZERO) {
        return;
      }
      auto coordinate = m_model->GetCoordinate(node);
      m_editVariant = new ReplaceNodeCommand(
        Ref(*m_model), coordinate, *node.SetValue(Money::ZERO));
      return;
    }
  }
  auto editor = new OptionalPriceSpinBox(Ref(*m_userProfile), node);
  if(keyEvent) {
    auto text = keyEvent->text();
    if(text.isEmpty() || !text[0].isLetterOrNumber()) {
      editor->SetValue(node.GetValue());
    } else {
      editor->clear();
    }
    QApplication::sendEvent(editor, m_event);
  } else {
    editor->SetValue(node.GetValue());
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const OrderStatusNode& node) {
  auto editor = new QComboBox();
  for(auto i = std::size_t(0); i < OrderStatus::COUNT; ++i) {
    editor->addItem(displayText(static_cast<OrderStatus>(i)));
  }
  editor->setCurrentIndex(static_cast<int>(node.GetValue()));
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const OrderTypeNode& node) {
  auto editor = new QComboBox();
  editor->addItem(QObject::tr("Limit"));
  editor->addItem(QObject::tr("Market"));
  editor->addItem(QObject::tr("Pegged"));
  editor->addItem(QObject::tr("Stop"));
  if(node.GetValue() == OrderType::LIMIT) {
    editor->setCurrentIndex(0);
  } else if(node.GetValue() == OrderType::MARKET) {
    editor->setCurrentIndex(1);
  } else if(node.GetValue() == OrderType::PEGGED) {
    editor->setCurrentIndex(2);
  } else if(node.GetValue() == OrderType::STOP) {
    editor->setCurrentIndex(3);
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const QueryNode& node) {
  auto editor = new QComboBox();
  auto& record =
    static_cast<const RecordType&>(node.GetChildren().front().GetType());
  if(IsRoot(node)) {
    for(auto& field : record.GetFields()) {
      editor->addItem(QString::fromStdString(field.m_name));
      if(field.m_name == node.GetField()) {
        editor->setCurrentIndex(editor->count() - 1);
      }
    }
  } else {
    for(auto& field : record.GetFields()) {
      if(field.m_name == node.GetField()) {
        editor->addItem(QString::fromStdString(field.m_name));
        editor->setCurrentIndex(editor->count() - 1);
      } else {
        auto builder = CanvasNodeBuilder(GetRoot(node));
        try {
          builder.Replace(node, node.SetField(field.m_name));
          auto replacement = builder.Make();
          if(!IsValueEqual(GetRoot(node), *replacement)) {
            editor->addItem(QString::fromStdString(field.m_name));
          }
        } catch(const std::exception&) {}
      }
    }
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const ReferenceNode& node) {
  auto editor = new QLineEdit();
  editor->setText(QString::fromStdString(node.GetReferent()));
  editor->selectAll();
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const SecurityNode& node) {
  auto widget = dynamic_cast<QWidget*>(m_model);
  auto dialog =
    new SecurityInputDialog(Ref(*m_userProfile), node.GetValue(), widget);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  if(m_event && m_event->type() == QEvent::KeyPress) {
    dialog->GetSymbolInput().selectAll();
    QApplication::sendEvent(
      &dialog->GetSymbolInput(), static_cast<QKeyEvent*>(m_event));
  }
  QObject::connect(dialog, &SecurityInputDialog::finished, widget,
    [=] (auto result) {
      if(result == QDialog::Rejected) {
        return;
      }
      auto newValue = dialog->GetSecurity();
      if(newValue == Security()) {
        return;
      }
      auto previousValue = node.GetValue();
      if(previousValue == newValue) {
        return;
      }
      auto coordinate = m_model->GetCoordinate(node);
      m_editVariant = new ReplaceNodeCommand(Ref(*m_model), coordinate,
        *node.SetValue(newValue, m_userProfile->GetMarketDatabase()));
    });
  dialog->show();
}

void OpenEditorCanvasNodeVisitor::Visit(const SideNode& node) {
  auto editor = new QComboBox();
  editor->addItem(QObject::tr("Bid"));
  editor->addItem(QObject::tr("Ask"));
  if(node.GetValue() == Side::ASK) {
    editor->setCurrentIndex(1);
  } else {
    editor->setCurrentIndex(0);
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const TextNode& node) {
  auto editor = new QLineEdit();
  editor->setText(QString::fromStdString(node.GetValue()));
  editor->selectAll();
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const TimeInForceNode& node) {
  auto editor = new QComboBox();
  for(auto i = 0; i < TimeInForce::Type::COUNT; ++i) {
    auto timeInForce = static_cast<TimeInForce::Type>(i);
    editor->addItem(
      QString::fromStdString(lexical_cast<std::string>(timeInForce)));
  }
  if(m_event) {
    QApplication::sendEvent(editor, m_event);
  }
  m_editVariant = editor;
}

void OpenEditorCanvasNodeVisitor::Visit(const TimeNode& node) {
  auto dialog = TimeInputDialog(
    node.GetValue(), Ref(*m_userProfile), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto previousValue = node.GetValue();
  auto newValue = dialog.GetTime();
  if(previousValue == newValue) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, *node.SetValue(newValue));
}

void OpenEditorCanvasNodeVisitor::Visit(const TimeRangeParameterNode& node) {
  auto dialog = TimeRangeInputDialog(
    node.GetStartTime(), node.GetEndTime(), dynamic_cast<QWidget*>(m_model));
  if(dialog.exec() == QDialog::Rejected) {
    return;
  }
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant = new ReplaceNodeCommand(Ref(*m_model), coordinate,
    *node.SetTimeRange(dialog.GetStartTime(), dialog.GetEndTime()));
}

void OpenEditorCanvasNodeVisitor::Visit(const CanvasNode& node) {
  auto catalogFilter = CatalogWindow::Filter();
  if(IsRoot(node)) {
    catalogFilter = CatalogWindow::DisplayAllFilter;
  } else {
    catalogFilter = CatalogWindow::SubstitutionFilter(node);
  }
  auto catalog = CatalogWindow(
    Ref(*m_userProfile), catalogFilter, dynamic_cast<QWidget*>(m_model));
  if(catalog.exec() == QDialog::Rejected) {
    return;
  }
  auto selectedEntry = catalog.GetSelection().front();
  auto coordinate = m_model->GetCoordinate(node);
  m_editVariant =
    new ReplaceNodeCommand(Ref(*m_model), coordinate, selectedEntry->GetNode());
}
