#include "Spire/InputWidgets/SelectionListInputDialog.hpp"
#include <algorithm>
#include "ui_SelectionListInputDialog.h"

using namespace Spire;
using namespace std;

SelectionListInputDialog::SelectionListInputDialog(
    const vector<string>& availableList, const vector<string>& selectedList,
    QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_SelectionListInputDialog>()),
      m_availableList(availableList) {
  m_ui->setupUi(this);
  setFixedSize(width(), height());
  for(vector<string>::const_iterator i = m_availableList.begin();
      i != m_availableList.end(); ++i) {
    if(std::find(selectedList.begin(), selectedList.end(), *i) ==
        selectedList.end()) {
      m_ui->m_availableList->addItem(QString::fromStdString(*i));
    }
  }
  for(vector<string>::const_iterator i = selectedList.begin();
      i != selectedList.end(); ++i) {
    m_ui->m_selectedList->addItem(QString::fromStdString(*i));
  }
  connect(m_ui->m_addButton, &QPushButton::clicked, this,
    &SelectionListInputDialog::OnAddItems);
  connect(m_ui->m_removeButton, &QPushButton::clicked, this,
    &SelectionListInputDialog::OnRemoveItems);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &SelectionListInputDialog::accept);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &SelectionListInputDialog::reject);
}

vector<string> SelectionListInputDialog::GetSelection() const {
  vector<string> selection;
  for(int i = 0; i < m_ui->m_selectedList->count(); ++i) {
    selection.push_back(m_ui->m_selectedList->item(i)->text().toStdString());
  }
  return selection;
}

void SelectionListInputDialog::OnAddItems() {
  QList<QListWidgetItem*> items = m_ui->m_availableList->selectedItems();
  for(QList<QListWidgetItem*>::const_iterator i = items.begin();
      i != items.end(); ++i) {
    m_ui->m_selectedList->addItem((*i)->text());
    m_ui->m_availableList->takeItem(m_ui->m_availableList->row(*i));
  }
}

void SelectionListInputDialog::OnRemoveItems() {
  QList<QListWidgetItem*> items = m_ui->m_selectedList->selectedItems();
  for(QList<QListWidgetItem*>::const_iterator i = items.begin();
      i != items.end(); ++i) {
    m_ui->m_availableList->addItem((*i)->text());
    m_ui->m_selectedList->takeItem(m_ui->m_selectedList->row(*i));
  }
}
