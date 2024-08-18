#include "Spire/CanvasView/LuaScriptDialog.hpp"
#include <QPushButton>
#include <QTableWidgetItem>
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_LuaScriptDialog.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  enum {
    NAME_COLUMN,
    TYPE_COLUMN,
  };

  const int COLUMN_COUNT = 2;
}

LuaScriptDialog::LuaScriptDialog(const LuaScriptNode& node,
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_LuaScriptDialog>()),
      m_userProfile(userProfile.Get()) {
  Setup(node);
}

LuaScriptDialog::LuaScriptDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_LuaScriptDialog>()),
      m_userProfile(userProfile.Get()) {
  LuaScriptNode luaScriptNode;
  Setup(luaScriptNode);
}

LuaScriptDialog::~LuaScriptDialog() {}

unique_ptr<LuaScriptNode> LuaScriptDialog::GetNode() {
  auto name = m_ui->m_nameInput->text().toStdString();
  auto scriptPath = m_ui->m_pathInput->GetPath();
  auto typeIndex = m_ui->m_typeComboBox->currentIndex();
  auto& type = static_cast<const NativeType&>(
    m_userProfile->GetCanvasTypeRegistry().GetTypes()[typeIndex]);
  auto node =
    std::make_unique<LuaScriptNode>(name, type, scriptPath, m_parameters);
  CanvasNodeBuilder builder(*node);
  for(std::size_t i = 0; i < m_nodes.size(); ++i) {
    builder.Replace(node->GetChildren()[i], CanvasNode::Clone(*m_nodes[i]));
  }
  return StaticCast<std::unique_ptr<LuaScriptNode>>(builder.Make());
}

void LuaScriptDialog::Setup(const LuaScriptNode& node) {
  m_ui->setupUi(this);
  m_ui->m_nameInput->setText(QString::fromStdString(node.GetName()));
  m_ui->m_nameInput->selectAll();
  m_ui->m_pathInput->SetPath(node.GetPath());
  m_ui->m_pathInput->SetCaption("Select Lua Script");
  m_ui->m_pathInput->SetFilter("Lua Script (*.lua)");
  auto types = m_userProfile->GetCanvasTypeRegistry().GetTypes();
  for(auto i = types.begin(); i != types.end(); ++i) {
    m_ui->m_typeComboBox->addItem(QString::fromStdString(i->GetName()));
    auto compatibility = i->GetCompatibility(node.GetType());
    if(compatibility == CanvasType::Compatibility::EQUAL) {
      m_ui->m_typeComboBox->setCurrentIndex(std::distance(types.begin(), i));
    }
  }
  m_ui->m_parametersTable->setColumnCount(COLUMN_COUNT);
  m_ui->m_parametersTable->setRowCount(
    static_cast<int>(node.GetChildren().size()) + 1);
  m_ui->m_parametersTable->setHorizontalHeaderLabels(QStringList() <<
    tr("Name") << tr("Type"));
  for(size_t i = 0; i < node.GetParameters().size(); ++i) {
    auto parameter = node.GetParameters()[i];
    auto child = CanvasNode::Clone(node.GetChildren()[i]);
    m_parameters.push_back(parameter);
    m_nodes.emplace_back(std::move(child));
  }
  for(auto i = m_parameters.begin(); i != m_parameters.end(); ++i) {
    AddParameter(*i, std::distance(m_parameters.begin(), i));
  }
  AddNewParameter();
  connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked, this,
    &LuaScriptDialog::OnButtonClicked);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &LuaScriptDialog::reject);
  m_itemChangedConnection = connect(m_ui->m_parametersTable,
    &LuaScriptParametersTable::itemChanged, this,
    &LuaScriptDialog::OnItemChanged);
  connect(m_ui->m_parametersTable, &LuaScriptParametersTable::itemActivated,
    this, &LuaScriptDialog::OnItemActivated);
  connect(m_ui->m_parametersTable,
    &LuaScriptParametersTable::currentItemChanged, this,
    &LuaScriptDialog::OnCurrentItemChanged);
}

void LuaScriptDialog::AddParameter(const LuaScriptNode::Parameter& parameter,
    int row) {
  auto nameItem =
    new QTableWidgetItem(QString::fromStdString(parameter.m_name));
  nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsDropEnabled);
  m_ui->m_parametersTable->setItem(row, NAME_COLUMN, nameItem);
  auto typeItem = new QTableWidgetItem(QString::fromStdString(
    parameter.m_type->GetName()));
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable &
    ~Qt::ItemIsDropEnabled);
  m_ui->m_parametersTable->setItem(row, TYPE_COLUMN, typeItem);
}

void LuaScriptDialog::DeleteParameter(QTableWidgetItem* item) {
  auto index = item->row();
  if(index == m_ui->m_parametersTable->rowCount() - 1) {
    return;
  }
  m_ui->m_parametersTable->removeRow(index);
  m_nodes.erase(m_nodes.begin() + index);
  m_parameters.erase(m_parameters.begin() + index);
}

void LuaScriptDialog::AddNewParameter() {
  auto newParameterItem = new QTableWidgetItem(tr("Add parameter."));
  newParameterItem->setFlags(newParameterItem->flags() &
    ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
  auto newParameterItemFont = newParameterItem->font();
  newParameterItemFont.setItalic(true);
  newParameterItem->setFont(newParameterItemFont);
  m_ui->m_parametersTable->setItem(m_ui->m_parametersTable->rowCount() - 1,
    NAME_COLUMN, newParameterItem);
  auto typeItem = new QTableWidgetItem();
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsDragEnabled &
    ~Qt::ItemIsDropEnabled & ~Qt::ItemIsEditable);
  m_ui->m_parametersTable->setItem(m_ui->m_parametersTable->rowCount() - 1,
    TYPE_COLUMN, typeItem);
}

void LuaScriptDialog::OnButtonClicked(QAbstractButton* button) {
  if(button == m_ui->m_buttonBox->button(QDialogButtonBox::Ok)) {
    accept();
  }
}

void LuaScriptDialog::OnItemChanged(QTableWidgetItem* item) {
  if(item->row() == m_ui->m_parametersTable->rowCount() - 1 &&
      item->column() == NAME_COLUMN) {
    disconnect(m_itemChangedConnection);
    LuaScriptNode::Parameter parameter(item->text().toStdString(),
      IntegerType::GetInstance());
    m_parameters.push_back(parameter);
    m_nodes.emplace_back(std::make_unique<NoneNode>(*parameter.m_type));
    auto typeItem = m_ui->m_parametersTable->item(item->row(), TYPE_COLUMN);
    typeItem->setText(QString::fromStdString(parameter.m_type->GetName()));
    typeItem->setFlags(typeItem->flags() | Qt::ItemIsDragEnabled);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setFont(font());
    m_ui->m_parametersTable->setRowCount(
      m_ui->m_parametersTable->rowCount() + 1);
    AddNewParameter();
    m_itemChangedConnection = connect(m_ui->m_parametersTable,
      &LuaScriptParametersTable::itemChanged, this,
      &LuaScriptDialog::OnItemChanged);
  } else if(item->column() == NAME_COLUMN) {
    m_parameters[item->row()].m_name = item->text().toStdString();
  }
}

void LuaScriptDialog::OnItemActivated(QTableWidgetItem* item) {
  if(m_ui->m_parametersTable->GetCanvasTypeComboBox() != nullptr ||
      item->column() != TYPE_COLUMN ||
      item->row() == m_ui->m_parametersTable->rowCount() - 1) {
    return;
  }
  m_ui->m_parametersTable->OpenTypeEditor(*item);
}

void LuaScriptDialog::OnCurrentItemChanged(QTableWidgetItem* current,
    QTableWidgetItem* previous) {
  if(m_ui->m_parametersTable->GetCanvasTypeComboBox() != nullptr) {
    m_ui->m_parametersTable->CommitTypeEditor(previous);
  }
}
