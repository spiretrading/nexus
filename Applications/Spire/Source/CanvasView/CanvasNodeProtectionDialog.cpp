#include "Spire/CanvasView/CanvasNodeProtectionDialog.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <QCheckBox>
#include <QEvent>
#include <QPushButton>
#include "Spire/Canvas/ReferenceNodes/ProxyNode.hpp"
#include "ui_CanvasNodeProtectionDialog.h"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  enum {
    NAME_COLUMN,
    HIDDEN_COLUMN,
    READ_ONLY_COLUMN,
  };

  static const int COLUMN_COUNT = 3;
}

CanvasNodeProtectionDialog::CanvasNodeProtectionDialog(const CanvasNode& node,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(make_unique<Ui_CanvasNodeProtectionDialog>()),
      m_node(CanvasNode::Clone(node)),
      m_builder(node) {
  m_ui->setupUi(this);
  QFontMetrics metrics(font());
  m_ui->m_protectionTable->verticalHeader()->setDefaultSectionSize(
    metrics.height() + 8);
  m_ui->m_protectionTable->horizontalHeader()->setSectionResizeMode(
    QHeaderView::ResizeToContents);
  m_ui->m_protectionTable->setColumnCount(COLUMN_COUNT);
  m_ui->m_protectionTable->setRowCount(
    static_cast<int>(m_node->GetChildren().size()));
  m_ui->m_protectionTable->setHorizontalHeaderLabels(QStringList() <<
    tr("Name") << tr("Hidden") << tr("Read-only"));
  auto children = m_node->GetChildren();
  size_t proxies = 0;
  for(auto i = children.begin(); i != children.end(); ++i) {
    if(dynamic_cast<const ProxyNode*>(&*i) != nullptr) {
      ++proxies;
      continue;
    }
    auto row = std::distance(children.begin(), i) - proxies;
    auto nodeName = i->GetName();
    replace_all(nodeName, "\\.", ".");
    auto nameItem = new QTableWidgetItem(QString::fromStdString(nodeName));
    m_ui->m_protectionTable->setItem(row, NAME_COLUMN, nameItem);
    auto hiddenWidget = new QWidget();
    auto hiddenLayout = new QHBoxLayout();
    hiddenLayout->setMargin(0);
    hiddenLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Expanding));
    auto hiddenCheckBox = new QCheckBox();
    hiddenCheckBox->setChecked(!(i->IsVisible()));
    hiddenCheckBox->setMaximumWidth(hiddenCheckBox->height());
    hiddenLayout->addWidget(hiddenCheckBox);
    hiddenLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Expanding));
    hiddenWidget->setLayout(hiddenLayout);
    m_ui->m_protectionTable->setCellWidget(row, HIDDEN_COLUMN, hiddenWidget);
    auto readOnlyWidget = new QWidget();
    auto readOnlyLayout = new QHBoxLayout();
    readOnlyLayout->setMargin(0);
    readOnlyLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Expanding));
    auto readOnlyCheckBox = new QCheckBox();
    readOnlyCheckBox->setChecked(i->IsReadOnly());
    readOnlyCheckBox->setMaximumWidth(readOnlyCheckBox->height());
    readOnlyLayout->addWidget(readOnlyCheckBox);
    readOnlyLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
      QSizePolicy::Expanding));
    readOnlyWidget->setLayout(readOnlyLayout);
    m_ui->m_protectionTable->setCellWidget(row, READ_ONLY_COLUMN,
      readOnlyWidget);
  }
  connect(m_ui->m_buttonBox, &QDialogButtonBox::clicked, this,
    &CanvasNodeProtectionDialog::OnButtonClicked);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &CanvasNodeProtectionDialog::reject);
}

CanvasNodeProtectionDialog::~CanvasNodeProtectionDialog() {}

unique_ptr<CanvasNode> CanvasNodeProtectionDialog::GetNode() {
  return m_builder.Make();
}

void CanvasNodeProtectionDialog::OnButtonClicked(QAbstractButton* button) {
  if(button == m_ui->m_buttonBox->button(QDialogButtonBox::Ok)) {
    auto children = m_node->GetChildren();
    size_t proxies = 0;
    for(auto i = children.begin(); i != children.end(); ++i) {
      if(dynamic_cast<const ProxyNode*>(&*i) != nullptr) {
        ++proxies;
        continue;
      }
      auto row = std::distance(children.begin(), i) - proxies;
      auto hiddenCheckBox = qobject_cast<QCheckBox*>(
        static_cast<QWidgetItem*>(m_ui->m_protectionTable->cellWidget(row,
        HIDDEN_COLUMN)->layout()->itemAt(1))->widget());
      m_builder.SetVisible(*i, !hiddenCheckBox->isChecked());
      auto readOnlyCheckBox = qobject_cast<QCheckBox*>(
        static_cast<QWidgetItem*>(m_ui->m_protectionTable->cellWidget(row,
        READ_ONLY_COLUMN)->layout()->itemAt(1))->widget());
      m_builder.SetReadOnly(*i, readOnlyCheckBox->isChecked());
    }
    accept();
  }
}
