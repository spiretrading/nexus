#include "Spire/Catalog/CatalogIconSelectionDialog.hpp"
#include "ui_CatalogIconSelectionDialog.h"

using namespace Spire;
using namespace std;

namespace {
  class IconListWidgetItem : public QListWidgetItem {
    public:
      IconListWidgetItem(const string& path, const string& name)
          : QListWidgetItem(QIcon(QString::fromStdString(path)),
              QString::fromStdString(name)),
            m_path(path) {}

      const string& GetPath() const {
        return m_path;
      }

    private:
      string m_path;
  };
}

CatalogIconSelectionDialog::CatalogIconSelectionDialog(QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_CatalogIconSelectionDialog>()) {
  m_ui->setupUi(this);
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/logic_and.png", "Conjunction"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(":/icons/weight.png",
    "Weight"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/date_time.png", "Date/Time"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(":/icons/target2.png",
    "Target"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/flash_yellow.png", "Execute"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(":/icons/money.png",
    "Money"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/sort_up_down.png", "Sort"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/bookmark_blue.png", "Bookmark"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/chart_line.png", "Chart"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/window_font.png", "Text"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/window_time.png", "Time"));
  m_ui->m_iconListWidget->addItem(new IconListWidgetItem(
    ":/icons/hourglass.png", "Hourglass"));
  connect(m_ui->m_iconListWidget, &QListWidget::itemActivated, this,
    &CatalogIconSelectionDialog::OnActivated);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::accepted, this,
    &CatalogIconSelectionDialog::OnAccept);
  connect(m_ui->m_buttonBox, &QDialogButtonBox::rejected, this,
    &CatalogIconSelectionDialog::reject);
}

CatalogIconSelectionDialog::~CatalogIconSelectionDialog() {}

const string& CatalogIconSelectionDialog::GetIconPath() const {
  QList<QListWidgetItem*> selectedItems =
    m_ui->m_iconListWidget->selectedItems();
  IconListWidgetItem* item = static_cast<IconListWidgetItem*>(
    selectedItems.front());
  return item->GetPath();
}

void CatalogIconSelectionDialog::OnActivated(QListWidgetItem* item) {
  Q_EMIT accept();
}

void CatalogIconSelectionDialog::OnAccept() {
  if(m_ui->m_iconListWidget->selectedItems().empty()) {
    return;
  }
  Q_EMIT accept();
}
