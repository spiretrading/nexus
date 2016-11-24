#include "Spire/Login/LoginDialog.hpp"
#include <Beam/ServiceLocator/AuthenticationException.hpp>
#include <Beam/ServiceLocator/SessionEncryption.hpp>
#include <boost/lexical_cast.hpp>
#include <QMouseEvent>
#include <QMovie>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include "Spire/Spire/ServiceClients.hpp"
#include "ui_LoginDialog.h"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace Beam::Threading;
using namespace boost;
using namespace Spire;
using namespace std;

namespace {
  const auto UPDATE_INTERVAL = 100;
}

LoginDialog::ServerInstance::ServerInstance(string name, IpAddress address)
  : m_name{std::move(name)},
    m_address{std::move(address)} {}

LoginDialog::LoginDialog(vector<ServerInstance> instances,
    RefType<SocketThreadPool> socketThreadPool,
    RefType<TimerThreadPool> timerThreadPool)
    : m_ui{std::make_unique<Ui_LoginDialog>()},
      m_instances{std::move(instances)},
      m_socketThreadPool{socketThreadPool.Get()},
      m_timerThreadPool{timerThreadPool.Get()},
      m_state{State::READY},
      m_loginCount{0} {
  setWindowFlags(Qt::FramelessWindowHint);
  m_ui->setupUi(this);
  auto screen = qApp->screens().at(0);
  auto physicalDotsPerInchY = screen->physicalDotsPerInchY();
  auto physicalDotsPerInchX = screen->physicalDotsPerInchX();
  QFont f{"Roboto", 10, QFont::Normal};
  const auto LOGIN_DIALOG_HEIGHT_INCHES = 3.6;
  const auto LOGIN_DIALOG_WIDTH_INCHES = 4.0;
  QPixmap pixmap{":/newPrefix/login-bg.png"};
  QBrush brush{pixmap.scaled(physicalDotsPerInchX * LOGIN_DIALOG_WIDTH_INCHES,
    LOGIN_DIALOG_HEIGHT_INCHES * physicalDotsPerInchY)};
  QPalette palette;
  palette.setBrush(QPalette::Background, brush);
  setPalette(palette);
  setFixedHeight(LOGIN_DIALOG_HEIGHT_INCHES * physicalDotsPerInchY);
  setFixedWidth(LOGIN_DIALOG_WIDTH_INCHES * physicalDotsPerInchX);
  const auto LEFT_SPACER_SIZE = 0.5;
  m_ui->m_leftSpacer->changeSize(LEFT_SPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  const auto RIGHT_SPACER_SIZE = 0.5;
  m_ui->m_rightSpacer->changeSize(RIGHT_SPACER_SIZE * physicalDotsPerInchX, 0,
    QSizePolicy::Fixed);
  m_ui->m_bottomSpacer->changeSize(0, 0.5 * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto USERNAME_INPUT_HEIGHT = 0.28;
  m_ui->m_usernameInput->setFixedHeight(USERNAME_INPUT_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_usernameInput->setFixedWidth(3.0 * physicalDotsPerInchX);
  const auto PASSWORD_INPUT_HEIGHT = 0.28;
  m_ui->m_usernameInput->setStyleSheet(
    "background-color:rgb(255, 255, 255);"
    "color: rgb(168, 168, 168);"
    "color: rgb(0, 0, 0);"
    "font: 10pt 'Roboto';");
  const auto LINE_EDIT_MARGIN = 0.1;
  m_ui->m_usernameInput->setTextMargins(LINE_EDIT_MARGIN *
    physicalDotsPerInchX,0, 0, 0);
  m_ui->m_passwordInput->setFixedHeight(PASSWORD_INPUT_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_passwordInput->setFixedWidth(2.76 * physicalDotsPerInchX);
  m_ui->m_passwordInput->setStyleSheet(
    "background-color: rgb(255, 255, 255);"
    "color: rgb(168, 168, 168);"
    "color: rgb(0, 0, 0);"
    "font: 10pt 'Roboto';");
  m_ui->m_passwordInput->setTextMargins(LINE_EDIT_MARGIN * physicalDotsPerInchX,
    0, 0, 0);
  m_ui->m_passwordInput->setEchoMode(QLineEdit::Password);
  const auto VERSION_LABEL_HEIGHT = 0.28;
  m_ui->m_versionLabel->setFixedHeight(VERSION_LABEL_HEIGHT *
    physicalDotsPerInchY);
  const auto VERSION_LABEL_WIDTH = 1.2;
  m_ui->m_versionLabel->setFixedWidth(
    VERSION_LABEL_WIDTH * physicalDotsPerInchX);
  m_ui->m_versionLabel->setStyleSheet("color:rgb(255,255,255)");
  m_ui->m_versionLabel->setText("Version 1.0.0");
  m_ui->m_versionLabel->setFont(f);
  const auto VERSION_LOGIN_SPACER_WIDTH = 0.3;
  m_ui->m_versionLoginSpacer->changeSize(VERSION_LOGIN_SPACER_WIDTH *
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  const auto LOADING_LABEL_HEIGHT = 0.28;
  m_ui->m_loadingLabel->setFixedHeight(LOADING_LABEL_HEIGHT *
    physicalDotsPerInchY);
  const auto LOADING_LABEL_WIDTH = 0.2288;
  m_ui->m_loadingLabel->setFixedWidth(
    LOADING_LABEL_WIDTH * physicalDotsPerInchX);
  m_ui->m_loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  const auto LOADING_LABEL_LOGIN_SPACER_WIDTH = 0.25;
  m_ui->m_loadingLabelLoginSpacer->changeSize(
    LOADING_LABEL_LOGIN_SPACER_WIDTH *
    physicalDotsPerInchX, 0, QSizePolicy::Fixed);
  const auto LOGIN_BUTTON_HEIGHT = 0.28;
  m_ui->m_loginButton->setFixedHeight(LOGIN_BUTTON_HEIGHT *
    physicalDotsPerInchY);
  m_ui->m_loginButton->setFixedWidth(physicalDotsPerInchX);
  m_ui->m_loginButton->setFont(f);
  m_ui->m_loginButton->setStyleSheet(
    "QPushButton{"
    "  background-color:rgb(104, 75, 199);"
    "  color: white;"
    "  border: 2px solid rgb(141,120,236);"
    "}"
    "QPushButton:hover{"
    "  background-color:rgb(141, 120, 236);"
    "  color: white;"
    "  border: 2px solid rgb(185,180,236);"
    "}"
    "QPushButton:pressed{"
    "  background-color:rgb(90, 58, 172);"
    "  color: white;"
    "  border: 2px solid rgb(104,75,199);"
    "}");
  const auto SPIRE_LOGO_LABEL_HEIGHT = 0.5;
  m_ui->m_spireLogoLabel->setFixedHeight(SPIRE_LOGO_LABEL_HEIGHT *
    physicalDotsPerInchY);
  const auto SPIRE_LOGO_LABEL_WIDTH = 1.44;
  m_ui->m_spireLogoLabel->setFixedWidth(SPIRE_LOGO_LABEL_WIDTH *
    physicalDotsPerInchX);
  m_ui->m_spireLogoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  m_ui->m_spireLogoLabel->setStyleSheet(
    "border-image - image: url(: / newPrefix / spire - white.png);");
  const auto SPIRE_LOGO_INVALID_SPACER_WIDTH = 0.15;
  m_ui->m_spireLogoInvalidSpacer->changeSize(0,
    SPIRE_LOGO_INVALID_SPACER_WIDTH * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  m_ui->m_mainLayout->setAlignment(Qt::AlignCenter);
  const auto LOGO_TOP_SPACER_HEIGHT = 0.8;
  m_ui->m_logoTopSpacer->changeSize(0, LOGO_TOP_SPACER_HEIGHT *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  const auto LOGO_SPACER_HEIGHT = 0.25;
  m_ui->m_logoSpacer->changeSize(0, LOGO_SPACER_HEIGHT * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto USERNAME_PASSWORD_SPACER_HEIGHT = 0.125;
  m_ui->m_usernamePasswordSpacer->changeSize(0,
    USERNAME_PASSWORD_SPACER_HEIGHT * physicalDotsPerInchY,
    QSizePolicy::Fixed);
  const auto PASSWORD_LOGIN_SPACER = 0.25;
  m_ui->m_passwordLoginSpacer->changeSize(0, PASSWORD_LOGIN_SPACER *
    physicalDotsPerInchY, QSizePolicy::Fixed);
  m_ui->m_invalidLabel->setAlignment(Qt::AlignHCenter);
  m_ui->m_invalidLabel->setFont(f);
  m_ui->m_invalidLabel->setStyleSheet(
    "QLabel { color: qrgba(255, 255, 255, 0); }");
  const auto PASSWORD_CONTAINER_HEIGHT = 0.28;
  m_ui->m_passwordContainer->setFixedSize(3 * physicalDotsPerInchX,
    PASSWORD_CONTAINER_HEIGHT * physicalDotsPerInchY);
  m_ui->m_passwordContainer->setStyleSheet("background-color:white");
  const auto CHROMA_WIDTH = 0.08;
  const auto CHROMA_HEIGHT = 0.24;
  m_ui->m_chroma1->setFixedSize(CHROMA_WIDTH * physicalDotsPerInchX,
    CHROMA_HEIGHT * physicalDotsPerInchY);
  m_ui->m_chroma2->setFixedSize(CHROMA_WIDTH * physicalDotsPerInchX,
    CHROMA_HEIGHT * physicalDotsPerInchY);
  m_ui->m_chroma3->setFixedSize(CHROMA_WIDTH * physicalDotsPerInchX,
    CHROMA_HEIGHT * physicalDotsPerInchY);
  connect(m_ui->m_loginButton, &QPushButton::clicked, this,
    &LoginDialog::HandleLoginButtonClicked);
  connect(m_ui->m_passwordInput, &QLineEdit::textChanged, this,
    &LoginDialog::HandlePasswordTextChanged);
  connect(&m_updateTimer, &QTimer::timeout, this,
    &LoginDialog::HandleUpdateTimer);
  UpdatePasswordColor();
  m_updateTimer.start(UPDATE_INTERVAL);
}

LoginDialog::~LoginDialog() {}

unique_ptr<ServiceClients> LoginDialog::GetServiceClients() {
  return std::move(m_serviceClients);
}

void LoginDialog::mousePressEvent(QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    m_mousePoint = event->pos();
  }
}

void LoginDialog::mouseMoveEvent(QMouseEvent* event) {
  if(event->buttons() & Qt::LeftButton) {
    auto diff = event->pos() - m_mousePoint;
    auto newpos = pos() + diff;
    move(newpos);
  }
}

void LoginDialog::SetReadyState(const QString& response) {
  m_ui->m_usernameInput->setEnabled(true);
  m_ui->m_passwordInput->setEnabled(true);
  m_ui->m_loginButton->setText(tr("Login"));
  auto loadingIcon = m_ui->m_loadingLabel->movie();
  loadingIcon->stop();
  m_ui->m_loadingLabel->clear();
  delete loadingIcon;
  m_ui->m_invalidLabel->setText(response);
  m_ui->m_invalidLabel->setStyleSheet(
    "background-color : rgba(0,0,0,0%);"
    "color:rgb(219, 213, 44)");
  ++m_loginCount;
  m_state = State::READY;
}

void LoginDialog::UpdatePasswordColor() {
  const auto CHROMA_HASHES = 3;
  const auto COLOR_LENGTH = 6;
  auto password = m_ui->m_passwordInput->text().toStdString();
  auto hash = ComputeSHA(password);
  for(auto i = 0; i < CHROMA_HASHES; ++i) {
    auto color = hash.substr(COLOR_LENGTH * i, COLOR_LENGTH);
    auto styleSheet = QString::fromStdString(
      "\
      QWidget#m_chroma" + lexical_cast<string>(i + 1) + " {\
        background-color: #" + color + ";\
      }\
    ");
    if(i == 0) {
      m_ui->m_chroma1->setStyleSheet(styleSheet);
    } else if(i == 1) {
      m_ui->m_chroma2->setStyleSheet(styleSheet);
    } else {
      m_ui->m_chroma3->setStyleSheet(styleSheet);
    }
  }
}

void LoginDialog::HandleLoginButtonClicked() {
  if (m_state == State::READY) {
    m_ui->m_invalidLabel->setStyleSheet(
      "QLabel { color: qrgba(255, 255, 255, 0); }");
    m_ui->m_usernameInput->setEnabled(false);
    m_ui->m_passwordInput->setEnabled(false);
    m_ui->m_loginButton->setText(tr("Cancel"));
    auto movie = new QMovie{ ":/newPrefix/spire_desktop_loading_icon.gif" };
    movie->setScaledSize(m_ui->m_loadingLabel->size());
    m_ui->m_loadingLabel->setMovie(movie);
    movie->start();
    connect(movie, SIGNAL(finished()), movie, SLOT(start()));
    m_state = State::LOGGING_IN;
    m_tasks.Push([=, loginCount = m_loginCount] {
      auto username = m_ui->m_usernameInput->text().toStdString();
      auto password = m_ui->m_passwordInput->text().toStdString();
      auto serviceLocatorClient =
        std::make_unique<ApplicationServiceLocatorClient>();
      std::unique_ptr<ServiceClients> serviceClients;
      try {
        serviceLocatorClient->BuildSession(m_instances.front().m_address,
          Ref(*m_socketThreadPool), Ref(*m_timerThreadPool));
        (*serviceLocatorClient)->SetCredentials(username, password);
        (*serviceLocatorClient)->Open();
        serviceClients = std::make_unique<ServiceClients>(
          std::move(serviceLocatorClient), Ref(*m_socketThreadPool),
          Ref(*m_timerThreadPool));
        serviceClients->Open();
      } catch (const AuthenticationException&) {
        m_slotHandler.Push([=] {
          if (loginCount != m_loginCount) {
            return;
          }
          HandleAuthenticationError();
        });
        return;
      } catch (const std::exception& e) {
        m_slotHandler.Push([=] {
          if (loginCount != m_loginCount) {
            return;
          }
          HandleConnectionError();
        });
        return;
      }
      m_slotHandler.Push([=, serviceClients = serviceClients.release()] {
        if (loginCount != m_loginCount) {
          return;
        }
        HandleSuccess(unique_ptr<ServiceClients>{serviceClients});
      });
    });
  } else {
    SetReadyState(tr("Login canceled."));
  }
}

void LoginDialog::HandleAuthenticationError() {
  SetReadyState(tr("Invalid username or password."));
}

void LoginDialog::HandleConnectionError() {
  SetReadyState(tr("Unable to connect to server."));
}

void LoginDialog::HandleSuccess(
    std::unique_ptr<ServiceClients> serviceClients){
  SetReadyState({});
  m_serviceClients = std::move(serviceClients);
  Q_EMIT accept();
}

void LoginDialog::HandlePasswordTextChanged(const QString& text) {
  UpdatePasswordColor();
}

void LoginDialog::HandleUpdateTimer() {
  HandleTasks(m_slotHandler);
}
