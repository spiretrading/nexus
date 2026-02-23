#include "Spire/BookView/BookViewPropertiesDialog.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/UI/UserProfile.hpp"
#include "ui_BookViewPropertiesDialog.h"

using namespace Beam;
using namespace Nexus;
using namespace Spire;
using namespace std;

BookViewPropertiesDialog::BookViewPropertiesDialog(
    Ref<UserProfile> userProfile, const Ticker& ticker,
    const BookViewProperties& properties, QWidget* parent,
    Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_ui(std::make_unique<Ui_BookViewPropertiesDialog>()),
      m_userProfile(userProfile.get()),
      m_ticker(ticker),
      m_properties(properties) {
  m_ui->setupUi(this);
  m_ui->m_levelsTab->Initialize(Ref(m_properties));
  m_ui->m_highlightsTab->Initialize(Ref(*m_userProfile), Ref(m_properties));
  if(m_ticker) {
    m_ui->m_interactionsTab->Initialize(Ref(*m_userProfile), m_ticker);
  } else {
    m_ui->m_optionsTab->removeTab(m_ui->m_optionsTab->count() - 1);
  }
  connect(m_ui->m_loadDefaultButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnLoadDefault);
  connect(m_ui->m_saveAsDefaultButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnSaveAsDefault);
  connect(m_ui->m_resetDefaultButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnResetDefault);
  connect(m_ui->m_okButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnOk);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::reject);
  connect(m_ui->m_applyButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnApply);
  connect(m_ui->m_applyToAllButton, &QPushButton::clicked, this,
    &BookViewPropertiesDialog::OnApplyToAll);
  Redisplay();
}

BookViewPropertiesDialog::~BookViewPropertiesDialog() {}

const BookViewProperties& BookViewPropertiesDialog::GetProperties() const {
  return m_properties;
}

void BookViewPropertiesDialog::Redisplay() {
  m_ui->m_levelsTab->Redisplay();
  m_ui->m_highlightsTab->Redisplay();
}

void BookViewPropertiesDialog::OnLoadDefault() {
  m_properties = m_userProfile->GetDefaultBookViewProperties();
  Redisplay();
}

void BookViewPropertiesDialog::OnSaveAsDefault() {
  m_userProfile->SetDefaultBookViewProperties(m_properties);
}

void BookViewPropertiesDialog::OnResetDefault() {
  m_userProfile->SetDefaultBookViewProperties(BookViewProperties::GetDefault());
}

void BookViewPropertiesDialog::OnOk() {
  OnApply();
  Q_EMIT accept();
}

void BookViewPropertiesDialog::OnApply() {
  if(m_ticker) {
    m_userProfile->GetInteractionProperties() =
      m_ui->m_interactionsTab->GetProperties();
  }
  BookViewWindow* window = dynamic_cast<BookViewWindow*>(parent());
  if(window == nullptr) {
    return;
  }
  window->SetProperties(m_properties);
}

void BookViewPropertiesDialog::OnApplyToAll() {
  if(m_ticker) {
    m_userProfile->GetInteractionProperties() =
      m_ui->m_interactionsTab->GetProperties();
  }
  QWidgetList widgets = QApplication::topLevelWidgets();
  for(auto i = widgets.begin(); i != widgets.end(); ++i) {
    BookViewWindow* window = dynamic_cast<BookViewWindow*>(*i);
    if(window != nullptr) {
      window->SetProperties(m_properties);
    }
  }
}
