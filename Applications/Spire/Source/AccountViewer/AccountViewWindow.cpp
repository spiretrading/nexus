#include "Spire/AccountViewer/AccountViewWindow.hpp"
#include <QItemDelegate>
#include <QMessageBox>
#include "Spire/AccountViewer/AccountEntitlementModel.hpp"
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/AccountViewer/AccountViewItem.hpp"
#include "Spire/AccountViewer/AccountViewModel.hpp"
#include "Spire/AccountViewer/ComplianceModel.hpp"
#include "Spire/AccountViewer/DirectoryItem.hpp"
#include "Spire/AccountViewer/GroupDirectoryItemWidget.hpp"
#include "Spire/AccountViewer/ManagerItem.hpp"
#include "Spire/AccountViewer/ManagerItemWidget.hpp"
#include "Spire/AccountViewer/RiskModel.hpp"
#include "Spire/AccountViewer/RootItem.hpp"
#include "Spire/AccountViewer/ServiceItem.hpp"
#include "Spire/AccountViewer/ServiceItemWidget.hpp"
#include "Spire/AccountViewer/TraderItem.hpp"
#include "Spire/AccountViewer/TraderItemWidget.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_AccountViewWindow.h"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Spire;
using namespace std;

namespace {
  class AccountViewItemDelegate : public QItemDelegate {
    public:
      AccountViewItemDelegate() = default;

      QSize sizeHint(const QStyleOptionViewItem& option,
          const QModelIndex& index) const {
        auto parentSizeHint = QItemDelegate::sizeHint(option, index);
        return QSize{parentSizeHint.width(), parentSizeHint.height() + 4};
      }
  };
}

AccountViewWindow::AccountViewWindow(Ref<UserProfile> userProfile,
    QWidget* parent, Qt::WindowFlags flags)
    : QFrame{parent, flags},
      m_ui{std::make_unique<Ui_AccountViewWindow>()},
      m_userProfile{userProfile.Get()},
      m_model{std::make_unique<AccountViewModel>(Ref(*m_userProfile))} {
  m_ui->setupUi(this);
  try {
    m_model->Load();
  } catch(const std::exception& e) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to access the administration server: ") + e.what());
  }
  m_ui->m_accountView->setModel(m_model.get());
  m_ui->m_accountView->setItemDelegate(new AccountViewItemDelegate{});
  m_ui->m_accountDetailsArea->setDisabled(true);
  auto accountListSize = 180;
  auto accountListDelta = m_ui->m_accountView->width() - accountListSize;
  auto infoSize = m_ui->m_accountDetailsArea->width() + accountListDelta;
  m_ui->m_detailsViewSplitter->setSizes(QList<int>() << accountListSize <<
    infoSize);
  connect(m_ui->m_accountView, &QTreeView::expanded, this,
    &AccountViewWindow::OnEntryExpanded);
  for(auto& root : m_model->GetRoots()) {
    m_ui->m_accountView->expand(m_model->GetIndex(*root));
  }
  connect(m_ui->m_accountView->selectionModel(),
    &QItemSelectionModel::currentChanged, this,
    &AccountViewWindow::OnCurrentChanged);
}

AccountViewWindow::~AccountViewWindow() {}

void AccountViewWindow::OnEntryExpanded(const QModelIndex& index) {
  m_model->Expand(index);
}

void AccountViewWindow::OnCurrentChanged(const QModelIndex& current,
    const QModelIndex& previous) {
  auto item = m_model->GetItem(current);
  if(item == nullptr) {
    delete m_ui->m_accountDetailsArea->takeWidget();
    return;
  }
  if(item->GetType() == AccountViewItem::Type::DIRECTORY) {
    auto directory = static_cast<DirectoryItem*>(item);
    if(directory->GetDirectoryType() == DirectoryItem::DirectoryType::GROUP) {
      auto complianceModel = std::make_shared<ComplianceModel>(
        Ref(*m_userProfile), directory->GetEntry());
      try {
        complianceModel->Load();
      } catch(const std::exception& e) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
          QObject::tr("Unable to load the compliance rules: ") + e.what());
      }
      auto groupDirectoryItemWidget = new GroupDirectoryItemWidget{
        Ref(*m_userProfile), directory->GetEntry(), complianceModel};
      m_ui->m_accountDetailsArea->setWidget(groupDirectoryItemWidget);
      m_ui->m_accountDetailsArea->setEnabled(true);
    } else {
      delete m_ui->m_accountDetailsArea->takeWidget();
    }
  } else if(item->GetType() == AccountViewItem::Type::TRADER) {
    auto trader = static_cast<TraderItem*>(item);
    auto infoModel = std::make_shared<AccountInfoModel>(Ref(*m_userProfile),
      trader->GetEntry());
    try {
      infoModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the account info: ") + e.what());
    }
    auto entitlementModel = std::make_shared<AccountEntitlementModel>(
      Ref(*m_userProfile), trader->GetEntry());
    try {
      entitlementModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the account's entitlements: ") + e.what());
    }
    auto riskModel = std::make_shared<RiskModel>(Ref(*m_userProfile),
      trader->GetEntry());
    try {
      riskModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the risk parameters: ") + e.what());
    }
    auto complianceModel = std::make_shared<ComplianceModel>(
      Ref(*m_userProfile), trader->GetEntry());
    try {
      complianceModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the compliance rules: ") + e.what());
    }
    auto traderItemWidget = new TraderItemWidget{Ref(*m_userProfile), infoModel,
      entitlementModel, riskModel, complianceModel};
    m_ui->m_accountDetailsArea->setWidget(traderItemWidget);
    m_ui->m_accountDetailsArea->setEnabled(true);
  } else if(item->GetType() == AccountViewItem::Type::MANAGER) {
    auto manager = static_cast<ManagerItem*>(item);
    auto infoModel = std::make_shared<AccountInfoModel>(Ref(*m_userProfile),
      manager->GetEntry());
    try {
      infoModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the account info: ") + e.what());
    }
    auto managerItemWidget = new ManagerItemWidget{Ref(*m_userProfile),
      infoModel};
    m_ui->m_accountDetailsArea->setWidget(managerItemWidget);
    m_ui->m_accountDetailsArea->setEnabled(true);
  } else if(item->GetType() == AccountViewItem::Type::SERVICE) {
    auto service = static_cast<ServiceItem*>(item);
    auto entitlementModel = std::make_shared<AccountEntitlementModel>(
      Ref(*m_userProfile), service->GetEntry());
    try {
      entitlementModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the account's entitlements: ") + e.what());
    }
    auto serviceItemWidget = new ServiceItemWidget{Ref(*m_userProfile),
      entitlementModel};
    m_ui->m_accountDetailsArea->setWidget(serviceItemWidget);
    m_ui->m_accountDetailsArea->setEnabled(true);
  } else if(m_userProfile->IsAdministrator() &&
      item->GetType() == AccountViewItem::Type::ROOT &&
      item->GetName() == "Trading Groups") {
    auto root = static_cast<RootItem*>(item);
    auto entry =
      [&] {
        auto& serviceLocatorClient =
          m_userProfile->GetServiceClients().GetServiceLocatorClient();
        auto groups = m_userProfile->GetServiceClients().
          GetAdministrationClient().LoadManagedTradingGroups(
          serviceLocatorClient.GetAccount());
        if(groups.empty()) {
          QMessageBox::critical(nullptr, QObject::tr("Error"),
            QObject::tr("Unable to load root directory."));
          return DirectoryEntry{};
        }
        auto parents = serviceLocatorClient.LoadParents(groups.front());
        if(parents.empty()) {
          return groups.front();
        }
        return parents.front();
      }();
    auto complianceModel = std::make_shared<ComplianceModel>(
      Ref(*m_userProfile), entry);
    try {
      complianceModel->Load();
    } catch(const std::exception& e) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load the compliance rules: ") + e.what());
    }
    auto groupDirectoryItemWidget = new GroupDirectoryItemWidget{
      Ref(*m_userProfile), entry, complianceModel};
    groupDirectoryItemWidget->RemoveActivityReportTab();
    m_ui->m_accountDetailsArea->setWidget(groupDirectoryItemWidget);
    m_ui->m_accountDetailsArea->setEnabled(true);
  } else {
    delete m_ui->m_accountDetailsArea->takeWidget();
  }
}
