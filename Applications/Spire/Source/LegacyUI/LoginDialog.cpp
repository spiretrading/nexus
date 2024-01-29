#include "Spire/LegacyUI/LoginDialog.hpp"
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include <boost/lexical_cast.hpp>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include "ui_LoginDialog.h"
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  const auto FOCUS_IN_STYLE = QString("\
    QWidget#m_usernameBox {\
      background-color: rgb(174, 203, 232);\
    }\
    QWidget#m_passwordBox {\
      background-color: rgb(174, 203, 232);\
    }\
    QWidget#m_serverBox {\
      background-color: rgb(174, 203, 232);\
    }\
  ");
}

LoginDialog::LoginDialog(std::vector<ServerEntry> servers)
    : m_ui(std::make_unique<Ui_LoginDialog>()),
      m_servers(std::move(servers)),
      m_state(State::READY) {
  m_ui->setupUi(this);
  for(auto& server : m_servers) {
    m_ui->m_serverComboBox->addItem(QString::fromStdString(server.m_name));
  }
  auto desktopGeometry = QGuiApplication::primaryScreen()->geometry();
  move(desktopGeometry.center() - rect().center());
  m_ui->m_usernameInput->installEventFilter(this);
  m_ui->m_passwordInput->installEventFilter(this);
  m_ui->m_serverComboBox->installEventFilter(this);
  m_ui->m_usernameInput->setFocus();
  connect(m_ui->m_passwordInput, &QLineEdit::textChanged, this,
    &LoginDialog::OnPasswordTextChanged);
  connect(m_ui->m_loginButton, &QPushButton::clicked, this,
    &LoginDialog::OnLoginButtonClicked);
  connect(m_ui->m_cancelButton, &QPushButton::clicked, this,
    &LoginDialog::OnCancelButtonClicked);
  UpdateChromaHash();
  setWindowTitle("Login - Version " SPIRE_VERSION);
}

LoginDialog::~LoginDialog() = default;

std::unique_ptr<ApplicationServiceLocatorClient>
    LoginDialog::GetServiceLocatorClient() {
  return std::move(m_serviceLocatorClient);
}

std::string LoginDialog::GetUsername() const {
  return m_ui->m_usernameInput->text().toStdString();
}

std::string LoginDialog::GetPassword() const {
  return m_ui->m_passwordInput->text().toStdString();
}

bool LoginDialog::eventFilter(QObject* object, QEvent* event) {
  if(object == m_ui->m_usernameInput && event->type() == QEvent::FocusIn) {
    m_ui->m_usernameBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_usernameInput &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_usernameBox->setStyleSheet("");
  } else if(object == m_ui->m_passwordInput &&
      event->type() == QEvent::FocusIn) {
    m_ui->m_passwordBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_passwordInput &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_passwordBox->setStyleSheet("");
  } else if(object == m_ui->m_serverComboBox &&
      event->type() == QEvent::FocusIn) {
    m_ui->m_serverBox->setStyleSheet(FOCUS_IN_STYLE);
  } else if(object == m_ui->m_serverComboBox &&
      event->type() == QEvent::FocusOut) {
    m_ui->m_serverBox->setStyleSheet("");
  }
  return QWidget::eventFilter(object, event);
}

void LoginDialog::UpdateChromaHash() {
  static const auto CHROMA_HASHES = 3;
  static const auto COLOR_LENGTH = 6;
  auto password = m_ui->m_passwordInput->text().toStdString();
  auto hash = ComputeSHA(password);
  for(auto i = 0; i < CHROMA_HASHES; ++i) {
    auto color = hash.substr(COLOR_LENGTH * i, COLOR_LENGTH);
    auto styleSheet = QString::fromStdString(
    "\
      QWidget#m_chromaHash" + lexical_cast<std::string>(i + 1) + " {\
        background-color: #" + color + ";\
      }\
    ");
    if(i == 0) {
      m_ui->m_chromaHash1->setStyleSheet(styleSheet);
    } else if(i == 1) {
      m_ui->m_chromaHash2->setStyleSheet(styleSheet);
    } else {
      m_ui->m_chromaHash3->setStyleSheet(styleSheet);
    }
  }
}

void LoginDialog::OnPasswordTextChanged(const QString& text) {
  UpdateChromaHash();
}

void LoginDialog::OnLoginButtonClicked() {
  m_ui->m_resultLabel->clear();
  m_ui->m_resultLabel->repaint();
  auto address = m_servers[m_ui->m_serverComboBox->currentIndex()].m_address;
  auto username = GetUsername();
  auto password = GetPassword();
  m_ui->m_usernameInput->setEnabled(false);
  m_ui->m_passwordInput->setEnabled(false);
  m_ui->m_serverComboBox->setEnabled(false);
  m_ui->m_loginButton->setEnabled(false);
  m_state = State::LOADING;
  m_loginPromise = QtPromise([=] {
    return std::make_unique<ApplicationServiceLocatorClient>(
      username, password, address);
  }, LaunchPolicy::ASYNC).then([=] (auto&& serviceLocatorClient) {
    m_ui->m_usernameInput->setEnabled(true);
    m_ui->m_passwordInput->setEnabled(true);
    m_ui->m_serverComboBox->setEnabled(true);
    m_ui->m_loginButton->setEnabled(true);
    m_state = State::READY;
    try {
      m_serviceLocatorClient = std::move(serviceLocatorClient.Get());
      Q_EMIT accept();
    } catch(const AuthenticationException&) {
      m_ui->m_resultLabel->setText(tr("Invalid username or password."));
    } catch(const std::exception&) {
      m_ui->m_resultLabel->setText(tr("Login connection failed."));
    }
  });
}

void LoginDialog::OnCancelButtonClicked() {
  if(m_state == State::READY) {
    Q_EMIT reject();
  } else {
    m_loginPromise = std::nullopt;
    m_state = State::READY;
    m_ui->m_usernameInput->setEnabled(true);
    m_ui->m_passwordInput->setEnabled(true);
    m_ui->m_serverComboBox->setEnabled(true);
    m_ui->m_loginButton->setEnabled(true);
    m_ui->m_resultLabel->setText(tr("Login canceled by user."));
  }
}
