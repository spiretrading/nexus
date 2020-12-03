#include "Spire/AccountViewer/AccountInfoWidget.hpp"
#include <QMessageBox>
#include "Spire/AccountViewer/AccountInfoModel.hpp"
#include "Spire/UI/CustomQtVariants.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_AccountInfoWidget.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

AccountInfoWidget::AccountInfoWidget(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags),
      m_ui(std::make_unique<Ui_AccountInfoWidget>()),
      m_isReadOnly(true) {
  m_ui->setupUi(this);
  connect(m_ui->m_updatePasswordButton, &QPushButton::clicked, this,
    &AccountInfoWidget::OnUpdatePasswordButton);
}

AccountInfoWidget::~AccountInfoWidget() {}

void AccountInfoWidget::Initialize(Ref<UserProfile> userProfile,
    bool isReadOnly, bool isPasswordReadOnly) {
  m_userProfile = userProfile.Get();
  m_isReadOnly = isReadOnly;
  m_ui->m_countryInput->clear();
  if(!m_isReadOnly) {
    const vector<CountryDatabase::Entry>& countries =
      m_userProfile->GetCountryDatabase().GetEntries();
    for(const CountryDatabase::Entry& country : countries) {
      m_ui->m_countryInput->addItem(QString::fromStdString(country.m_name));
    }
  }
  m_ui->m_changePhotoButton->setEnabled(!isReadOnly);
  m_ui->m_newPasswordInput->setEnabled(!isPasswordReadOnly);
  m_ui->m_confirmPasswordInput->setEnabled(!isPasswordReadOnly);
  m_ui->m_updatePasswordButton->setEnabled(!isPasswordReadOnly);
  m_ui->m_userNotesInput->setReadOnly(isReadOnly);
  m_ui->m_firstNameInput->setReadOnly(isReadOnly);
  m_ui->m_lastNameInput->setReadOnly(isReadOnly);
  m_ui->m_emailInput->setReadOnly(isReadOnly);
  m_ui->m_addressLineOneInput->setReadOnly(isReadOnly);
  m_ui->m_addressLineTwoInput->setReadOnly(isReadOnly);
  m_ui->m_addressLineThreeInput->setReadOnly(isReadOnly);
  m_ui->m_cityInput->setReadOnly(isReadOnly);
  m_ui->m_provinceInput->setReadOnly(isReadOnly);
}

const AccountInfoModel& AccountInfoWidget::GetModel() const {
  return *m_model;
}

AccountInfoModel& AccountInfoWidget::GetModel() {
  return *m_model;
}

void AccountInfoWidget::SetModel(
    const std::shared_ptr<AccountInfoModel>& model) {
  m_model = model;
  m_ui->m_firstNameInput->setText(
    QString::fromStdString(model->GetIdentity().m_firstName));
  m_ui->m_lastNameInput->setText(
    QString::fromStdString(model->GetIdentity().m_lastName));
  m_ui->m_lastLoginInput->setText(
    CustomVariantItemDelegate(Ref(*m_userProfile)).displayText(
    QVariant::fromValue(model->GetIdentity().m_lastLoginTime), QLocale()));
  m_ui->m_registeredInput->setText(
    CustomVariantItemDelegate(Ref(*m_userProfile)).displayText(
    QVariant::fromValue(model->GetIdentity().m_registrationTime), QLocale()));
  m_ui->m_accountIdInput->setText(QString::number(model->GetAccount().m_id));
  m_ui->m_emailInput->setText(QString::fromStdString(
    model->GetIdentity().m_emailAddress));
  m_ui->m_addressLineOneInput->setText(
    QString::fromStdString(model->GetIdentity().m_addressLineOne));
  m_ui->m_addressLineTwoInput->setText(
    QString::fromStdString(model->GetIdentity().m_addressLineTwo));
  m_ui->m_addressLineThreeInput->setText(
    QString::fromStdString(model->GetIdentity().m_addressLineThree));
  m_ui->m_cityInput->setText(
    QString::fromStdString(model->GetIdentity().m_city));
  m_ui->m_provinceInput->setText(
    QString::fromStdString(model->GetIdentity().m_province));
  m_ui->m_userNotesInput->setPlainText(
    QString::fromStdString(model->GetIdentity().m_userNotes));
  const CountryDatabase::Entry& country =
    m_userProfile->GetCountryDatabase().FromCode(
    m_model->GetIdentity().m_country);
  if(m_isReadOnly) {
    m_ui->m_countryInput->clear();
    m_ui->m_countryInput->addItem(QString::fromStdString(country.m_name));
  } else {
    for(int i = 0; i < m_ui->m_countryInput->count(); ++i) {
      if(m_ui->m_countryInput->itemText(i).toStdString() == country.m_name) {
        m_ui->m_countryInput->setCurrentIndex(i);
        break;
      }
    }
  }
}

void AccountInfoWidget::Commit() {
  m_model->GetIdentity().m_firstName =
    m_ui->m_firstNameInput->text().toStdString();
  m_model->GetIdentity().m_lastName =
    m_ui->m_lastNameInput->text().toStdString();
  m_model->GetIdentity().m_emailAddress =
    m_ui->m_emailInput->text().toStdString();
  m_model->GetIdentity().m_addressLineOne =
    m_ui->m_addressLineOneInput->text().toStdString();
  m_model->GetIdentity().m_addressLineTwo =
    m_ui->m_addressLineTwoInput->text().toStdString();
  m_model->GetIdentity().m_addressLineThree =
    m_ui->m_addressLineThreeInput->text().toStdString();
  m_model->GetIdentity().m_city = m_ui->m_cityInput->text().toStdString();
  m_model->GetIdentity().m_province =
    m_ui->m_provinceInput->text().toStdString();
  m_model->GetIdentity().m_userNotes =
    m_ui->m_userNotesInput->toPlainText().toStdString();
  const CountryDatabase::Entry& country =
    m_userProfile->GetCountryDatabase().FromName(
    m_ui->m_countryInput->currentText().toStdString());
  m_model->GetIdentity().m_country = country.m_code;
  try {
    m_model->Commit();
  } catch(std::exception&) {
    QMessageBox::critical(this, QObject::tr("Error"),
      QObject::tr("Unable to save the account info."));
  }
}

void AccountInfoWidget::OnUpdatePasswordButton() {
  if(m_ui->m_newPasswordInput->text().isEmpty()) {
    QMessageBox::critical(this, QObject::tr("Error"),
      QObject::tr("Password field can not be empty."));
    m_ui->m_newPasswordInput->clear();
    m_ui->m_confirmPasswordInput->clear();
    return;
  }
  if(m_ui->m_newPasswordInput->text() != m_ui->m_confirmPasswordInput->text()) {
    QMessageBox::critical(this, QObject::tr("Error"),
      QObject::tr("Passwords entered do not match."));
    m_ui->m_newPasswordInput->clear();
    m_ui->m_confirmPasswordInput->clear();
    return;
  }
  string password = m_ui->m_newPasswordInput->text().toStdString();
  try {
    m_userProfile->GetServiceClients().GetServiceLocatorClient().StorePassword(
      m_model->GetAccount(), password);
    QMessageBox::information(this, QObject::tr("Spire"),
      QObject::tr("Password has been updated successfully."));
  } catch(std::exception& e) {
    QMessageBox::critical(this, QObject::tr("Error"),
      QString::fromStdString(e.what()));
  }
  m_ui->m_newPasswordInput->clear();
  m_ui->m_confirmPasswordInput->clear();
}
