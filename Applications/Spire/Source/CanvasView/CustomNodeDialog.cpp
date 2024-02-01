#include "Spire/CanvasView/CustomNodeDialog.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <QPushButton>
#include <QTableWidgetItem>
#include "Spire/Canvas/Common/NoneNode.hpp"
#include "Spire/Canvas/Operations/CanvasNodeBuilder.hpp"
#include "Spire/Canvas/Types/UnionType.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "ui_CustomNodeDialog.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  enum {
    NAME_COLUMN,
    TYPE_COLUMN,
  };

  static const int COLUMN_COUNT = 2;
}

CustomNodeDialog::CustomNodeDialog(const CustomNode& customNode,
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_CustomNodeDialog>()),
      m_userProfile(userProfile.Get()) {
  Setup(customNode);
}

CustomNodeDialog::CustomNodeDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_CustomNodeDialog>()),
      m_userProfile(userProfile.Get()) {
  vector<CustomNode::Child> children;
  children.push_back(CustomNode::Child("expression", UnionType::GetAnyType()));
  CustomNode customNode("New Node", children);
  Setup(customNode);
}

CustomNodeDialog::~CustomNodeDialog() {}

unique_ptr<CustomNode> CustomNodeDialog::GetNode() {
  for(auto& child : m_children) {
    replace_all(child.m_name, ".", "\\.");
  }
  auto customNode = make_unique<CustomNode>(
    m_ui->m_nameInput->text().toStdString(), m_children);
  CanvasNodeBuilder builder(*customNode);
  for(std::size_t i = 0; i < m_nodes.size(); ++i) {
    builder.Replace(customNode->GetChildren()[i],
      CanvasNode::Clone(*m_nodes[i]));
  }
  return StaticCast<std::unique_ptr<CustomNode>>(builder.Make());
}

void CustomNodeDialog::Setup(const CustomNode& customNode) {
  m_ui->setupUi(this);
  m_ui->m_nameInput->setText(QString::fromStdString(customNode.GetText()));
  m_ui->m_nameInput->selectAll();
  m_ui->m_childrenTable->setColumnCount(COLUMN_COUNT);
  m_ui->m_childrenTable->setRowCount(
    static_cast<int>(customNode.GetChildren().size()) + 1);
  m_ui->m_childrenTable->setHorizontalHeaderLabels(QStringList() <<
    tr("Name") << tr("Type"));
  for(size_t i = 0; i < customNode.GetChildrenDetails().size(); ++i) {
    auto child = customNode.GetChildrenDetails()[i];
    auto node = CanvasNode::Clone(customNode.GetChildren()[i]);
    replace_all(child.m_name, "\\.", ".");
    m_children.push_back(child);
    m_nodes.emplace_back(std::move(node));
  }
  for(auto i = m_children.begin(); i != m_children.end(); ++i) {
    AddItem(*i, std::distance(m_children.begin(), i));
  }
  AddNewChildItem();
  connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked, this,
    &CustomNodeDialog::OnButtonClicked);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &CustomNodeDialog::reject);
  m_itemChangedConnection = connect(m_ui->m_childrenTable,
    &CustomNodeChildrenTable::itemChanged, this,
    &CustomNodeDialog::OnItemChanged);
  connect(m_ui->m_childrenTable, &CustomNodeChildrenTable::itemActivated, this,
    &CustomNodeDialog::OnItemActivated);
  connect(m_ui->m_childrenTable, &CustomNodeChildrenTable::currentItemChanged,
    this, &CustomNodeDialog::OnCurrentItemChanged);
}

void CustomNodeDialog::AddItem(const CustomNode::Child& child, int row) {
  auto nameItem = new QTableWidgetItem(QString::fromStdString(child.m_name));
  nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsDropEnabled);
  m_ui->m_childrenTable->setItem(row, NAME_COLUMN, nameItem);
  auto typeItem = new QTableWidgetItem(QString::fromStdString(
    child.m_type->GetName()));
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable &
    ~Qt::ItemIsDropEnabled);
  m_ui->m_childrenTable->setItem(row, TYPE_COLUMN, typeItem);
  if(row == 0) {
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsDragEnabled &
      ~Qt::ItemIsDropEnabled);
    typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsDragEnabled &
      ~Qt::ItemIsDropEnabled);
  }
}

void CustomNodeDialog::DeleteItem(QTableWidgetItem* item) {
  auto index = item->row();
  if(index == m_ui->m_childrenTable->rowCount() - 1) {
    return;
  }
  if(index == 0) {
    return;
  }
  m_ui->m_childrenTable->removeRow(index);
  m_nodes.erase(m_nodes.begin() + index);
  m_children.erase(m_children.begin() + index);
}

void CustomNodeDialog::AddNewChildItem() {
  auto newChildItem = new QTableWidgetItem(tr("Add child node."));
  newChildItem->setFlags(newChildItem->flags() & ~Qt::ItemIsDragEnabled &
    ~Qt::ItemIsDropEnabled);
  auto newChildItemFont = newChildItem->font();
  newChildItemFont.setItalic(true);
  newChildItem->setFont(newChildItemFont);
  m_ui->m_childrenTable->setItem(m_ui->m_childrenTable->rowCount() - 1,
    NAME_COLUMN, newChildItem);
  auto typeItem = new QTableWidgetItem();
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsDragEnabled &
    ~Qt::ItemIsDropEnabled & ~Qt::ItemIsEditable);
  m_ui->m_childrenTable->setItem(m_ui->m_childrenTable->rowCount() - 1,
    TYPE_COLUMN, typeItem);
}

void CustomNodeDialog::OnButtonClicked(QAbstractButton* button) {
  if(button == m_ui->m_buttonBox->button(QDialogButtonBox::Ok)) {
    accept();
  }
}

void CustomNodeDialog::OnItemChanged(QTableWidgetItem* item) {
  if(item->row() == m_ui->m_childrenTable->rowCount() - 1 &&
      item->column() == NAME_COLUMN) {
    disconnect(m_itemChangedConnection);
    CustomNode::Child child(item->text().toStdString(),
      UnionType::GetAnyType());
    m_children.push_back(child);
    m_nodes.emplace_back(make_unique<NoneNode>(*child.m_type));
    auto typeItem = m_ui->m_childrenTable->item(item->row(), TYPE_COLUMN);
    typeItem->setText(QString::fromStdString(child.m_type->GetName()));
    typeItem->setFlags(typeItem->flags() | Qt::ItemIsDragEnabled);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setFont(font());
    m_ui->m_childrenTable->setRowCount(m_ui->m_childrenTable->rowCount() + 1);
    AddNewChildItem();
    m_itemChangedConnection = connect(m_ui->m_childrenTable,
      &CustomNodeChildrenTable::itemChanged, this,
      &CustomNodeDialog::OnItemChanged);
  } else if(item->column() == NAME_COLUMN) {
    m_children[item->row()].m_name = item->text().toStdString();
  }
}

void CustomNodeDialog::OnItemActivated(QTableWidgetItem* item) {
  if(m_ui->m_childrenTable->GetCanvasTypeComboBox() != nullptr ||
      item->column() != TYPE_COLUMN ||
      item->row() == m_ui->m_childrenTable->rowCount() - 1) {
    return;
  }
  m_ui->m_childrenTable->OpenTypeEditor(*item);
}

void CustomNodeDialog::OnCurrentItemChanged(QTableWidgetItem* current,
    QTableWidgetItem* previous) {
  if(m_ui->m_childrenTable->GetCanvasTypeComboBox() != nullptr) {
    m_ui->m_childrenTable->CommitTypeEditor(previous);
  }
}
