#include "Spire/CanvasView/FileReaderNodeDialog.hpp"
#include <QPushButton>
#include <QTableWidgetItem>
#include "Spire/Canvas/IONodes/FileReaderNode.hpp"
#include "Spire/Canvas/Types/IntegerType.hpp"
#include "ui_FileReaderNodeDialog.h"

using namespace Beam;
using namespace Beam::Parsers;
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

FileReaderNodeDialog::FileReaderNodeDialog(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : m_ui(std::make_unique<Ui_FileReaderNodeDialog>()),
      m_userProfile(userProfile.Get()) {
  Setup(FileReaderNode());
}

FileReaderNodeDialog::FileReaderNodeDialog(const FileReaderNode& node,
    Ref<UserProfile> userProfile, QWidget* parent, Qt::WindowFlags flags)
    : m_ui(std::make_unique<Ui_FileReaderNodeDialog>()),
      m_userProfile(userProfile.Get()) {
  Setup(node);
}

FileReaderNodeDialog::~FileReaderNodeDialog() {}

unique_ptr<FileReaderNode> FileReaderNodeDialog::GetNode() {
  std::shared_ptr<NativeType> readType;
  if(m_fields.empty()) {
    FileReaderNode baseNode;
    auto emptyFileReader = StaticCast<std::unique_ptr<FileReaderNode>>(
      baseNode.Replace(baseNode.GetChildren().front(),
      CanvasNode::Clone(m_originalNode->GetChildren().front())));
    return emptyFileReader;
  } else if(m_fields.size() == 1) {
    readType = m_fields.front().m_type;
  } else {
    readType = MakeRecordType(m_fields);
  }
  return m_originalNode->SetReadType(*readType)->SetErrorPolicy(
    static_cast<ParserErrorPolicy>(
    m_ui->m_errorHandlingComboBox->currentIndex()));
}

void FileReaderNodeDialog::Setup(const FileReaderNode& node) {
  m_ui->setupUi(this);
  m_ui->m_errorHandlingComboBox->addItem("Report failure.");
  m_ui->m_errorHandlingComboBox->addItem("Skip error.");
  m_ui->m_errorHandlingComboBox->addItem("Stop reading.");
  m_ui->m_errorHandlingComboBox->setCurrentIndex(
    static_cast<int>(node.GetErrorPolicy()));
  m_originalNode = CanvasNode::Clone(node);
  m_ui->m_structureTable->setColumnCount(COLUMN_COUNT);
  m_ui->m_structureTable->setHorizontalHeaderLabels(QStringList() <<
    tr("Name") << tr("Type"));
  auto type = node.GetReadType();
  if(!type.is_initialized()) {
    m_ui->m_structureTable->setRowCount(1);
  } else {
    auto recordType = dynamic_cast<const RecordType*>(&*type);
    if(recordType != nullptr && recordType->GetName() == "Record") {
      m_ui->m_structureTable->setRowCount(
        static_cast<int>(recordType->GetFields().size()) + 1);
      for(const auto& field : recordType->GetFields()) {
        m_fields.push_back(field);
      }
    } else {
      m_ui->m_structureTable->setRowCount(2);
      m_fields.push_back(RecordType::Field("field", *type));
    }
  }
  for(auto i = m_fields.begin(); i != m_fields.end(); ++i) {
    AddField(*i, std::distance(m_fields.begin(), i));
  }
  AddNewField();
  connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked, this,
    &FileReaderNodeDialog::OnButtonClicked);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &FileReaderNodeDialog::reject);
  m_itemChangedConnection = connect(m_ui->m_structureTable,
    &FileReaderNodeStructureTable::itemChanged, this,
    &FileReaderNodeDialog::OnItemChanged);
  connect(m_ui->m_structureTable, &FileReaderNodeStructureTable::itemActivated,
    this, &FileReaderNodeDialog::OnItemActivated);
  connect(m_ui->m_structureTable,
    &FileReaderNodeStructureTable::currentItemChanged, this,
    &FileReaderNodeDialog::OnCurrentItemChanged);
}

void FileReaderNodeDialog::AddField(const RecordType::Field& field, int row) {
  auto nameItem = new QTableWidgetItem(QString::fromStdString(field.m_name));
  nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsDropEnabled);
  m_ui->m_structureTable->setItem(row, NAME_COLUMN, nameItem);
  QTableWidgetItem* typeItem = new QTableWidgetItem(
    QString::fromStdString(field.m_type->GetName()));
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable &
    ~Qt::ItemIsDropEnabled);
  m_ui->m_structureTable->setItem(row, TYPE_COLUMN, typeItem);
  if(row == 0) {
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsDragEnabled &
      ~Qt::ItemIsDropEnabled);
    typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsDragEnabled &
      ~Qt::ItemIsDropEnabled);
  }
}

void FileReaderNodeDialog::DeleteItem(QTableWidgetItem* item) {
  auto index = item->row();
  if(m_ui->m_structureTable->rowCount() > 1 &&
      index == m_ui->m_structureTable->rowCount() - 1) {
    return;
  }
  if(index == 0 && m_fields.empty()) {
    return;
  }
  m_ui->m_structureTable->removeRow(index);
  m_fields.erase(m_fields.begin() + index);
}

void FileReaderNodeDialog::AddNewField() {
  auto newFieldItem = new QTableWidgetItem(tr("Add field."));
  newFieldItem->setFlags(newFieldItem->flags() & ~Qt::ItemIsDragEnabled &
    ~Qt::ItemIsDropEnabled);
  auto newFieldItemFont = newFieldItem->font();
  newFieldItemFont.setItalic(true);
  newFieldItem->setFont(newFieldItemFont);
  m_ui->m_structureTable->setItem(m_ui->m_structureTable->rowCount() - 1,
    NAME_COLUMN, newFieldItem);
  auto typeItem = new QTableWidgetItem();
  typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsDragEnabled &
    ~Qt::ItemIsDropEnabled & ~Qt::ItemIsEditable);
  m_ui->m_structureTable->setItem(m_ui->m_structureTable->rowCount() - 1,
    TYPE_COLUMN, typeItem);
}

void FileReaderNodeDialog::OnButtonClicked(QAbstractButton* button) {
  if(button == m_ui->m_buttonBox->button(QDialogButtonBox::Ok)) {
    accept();
  }
}

void FileReaderNodeDialog::OnItemChanged(QTableWidgetItem* item) {
  if((m_fields.empty() ||
      item->row() == m_ui->m_structureTable->rowCount() - 1) &&
      item->column() == NAME_COLUMN) {
    disconnect(m_itemChangedConnection);
    RecordType::Field field(item->text().toStdString(),
      IntegerType::GetInstance());
    m_fields.push_back(field);
    auto typeItem = m_ui->m_structureTable->item(item->row(), TYPE_COLUMN);
    typeItem->setText(QString::fromStdString(field.m_type->GetName()));
    typeItem->setFlags(typeItem->flags() | Qt::ItemIsDragEnabled);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setFont(font());
    m_ui->m_structureTable->setRowCount(m_ui->m_structureTable->rowCount() + 1);
    AddNewField();
    m_itemChangedConnection = connect(m_ui->m_structureTable,
      &FileReaderNodeStructureTable::itemChanged, this,
      &FileReaderNodeDialog::OnItemChanged);
  } else if(item->column() == NAME_COLUMN) {
    m_fields[item->row()].m_name = item->text().toStdString();
  }
}

void FileReaderNodeDialog::OnItemActivated(QTableWidgetItem* item) {
  if(m_ui->m_structureTable->GetCanvasTypeComboBox() != nullptr ||
      item->column() != TYPE_COLUMN ||
      item->row() == m_ui->m_structureTable->rowCount() - 1) {
    return;
  }
  m_ui->m_structureTable->OpenTypeEditor(*item);
}

void FileReaderNodeDialog::OnCurrentItemChanged(QTableWidgetItem* current,
    QTableWidgetItem* previous) {
  if(m_ui->m_structureTable->GetCanvasTypeComboBox() != nullptr) {
    m_ui->m_structureTable->CommitTypeEditor(previous);
  }
}
