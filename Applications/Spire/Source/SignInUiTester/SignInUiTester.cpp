#include "Spire/SignInUiTester/SignInUiTester.hpp"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/SignIn/SignInWindow.hpp"
#include "Version.hpp"

using namespace Spire;

SignInUiTester::SignInUiTester()
    : m_is_closing(false),
      m_track(std::make_shared<LocalTrackModel>(Track::CURRENT)),
      m_window(nullptr) {
  setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
  auto layout = new QVBoxLayout(this);
  auto track_group = new QGroupBox(tr("Tracks"));
  auto track_layout = new QVBoxLayout(track_group);
  m_preview_check_box = new QCheckBox(tr("Preview"));
  track_layout->addWidget(m_preview_check_box);
  m_classic_check_box = new QCheckBox(tr("Classic"));
  track_layout->addWidget(m_classic_check_box);
  layout->addWidget(track_group);
  auto server_group = new QGroupBox(tr("Servers"));
  auto server_layout = new QVBoxLayout(server_group);
  m_live_trading_check_box = new QCheckBox(tr("Live Trading"));
  server_layout->addWidget(m_live_trading_check_box);
  m_staging_check_box = new QCheckBox(tr("Staging"));
  server_layout->addWidget(m_staging_check_box);
  m_uat_environment_check_box = new QCheckBox(tr("UAT Environment"));
  server_layout->addWidget(m_uat_environment_check_box);
  m_local_environment_check_box = new QCheckBox(tr("Local Environment"));
  server_layout->addWidget(m_local_environment_check_box);
  layout->addWidget(server_group);
  auto state_group = new QGroupBox(tr("State"));
  auto state_layout = new QHBoxLayout(state_group);
  m_accept_button = new QPushButton(tr("Accept"));
  m_accept_button->setDisabled(true);
  connect(
    m_accept_button, &QPushButton::pressed, this, &SignInUiTester::on_accept);
  state_layout->addWidget(m_accept_button);
  m_reject_button = new QPushButton(tr("Reject"));
  m_reject_button->setDisabled(true);
  connect(
    m_reject_button, &QPushButton::pressed, this, &SignInUiTester::on_reject);
  state_layout->addWidget(m_reject_button);
  m_error_button = new QPushButton(tr("Error"));
  m_error_button->setDisabled(true);
  connect(
    m_error_button, &QPushButton::pressed, this, &SignInUiTester::on_error);
  state_layout->addWidget(m_error_button);
  layout->addWidget(state_group);
  m_signals_text = new QTextEdit();
  m_signals_text->setReadOnly(true);
  m_signals_text->setFixedHeight(100);
  m_signals_text->setSizePolicy(
    QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  layout->addWidget(m_signals_text);
  m_rebuild_button = new QPushButton(tr("Rebuild"));
  connect(
    m_rebuild_button, &QPushButton::pressed, this, &SignInUiTester::on_rebuild);
  layout->addWidget(m_rebuild_button);
  on_rebuild();
  move(
    m_window->pos().x() + std::abs(m_window->width() - sizeHint().width()) / 2,
    m_window->pos().y() + m_window->height() + 5);
}

bool SignInUiTester::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    if(!m_is_closing) {
      close();
    }
  }
  return QWidget::eventFilter(watched, event);
}

void SignInUiTester::on_accept() {
  m_window->set_state(SignInWindow::State::NONE);
  m_accept_button->setDisabled(true);
  m_reject_button->setDisabled(true);
  m_error_button->setDisabled(true);
}

void SignInUiTester::on_reject() {
  m_window->set_error(tr("Incorrect username or password."));
  m_accept_button->setDisabled(true);
  m_reject_button->setDisabled(true);
  m_error_button->setDisabled(true);
}

void SignInUiTester::on_error() {
  m_window->set_error(tr("Server unavailable."));
  m_accept_button->setDisabled(true);
  m_reject_button->setDisabled(true);
  m_error_button->setDisabled(true);
}

void SignInUiTester::on_rebuild() {
  m_is_closing = true;
  delete m_window;
  m_is_closing = false;
  auto tracks = std::vector<Track>();
  tracks.push_back(Track::CURRENT);
  if(m_preview_check_box->isChecked()) {
    tracks.push_back(Track::PREVIEW);
  }
  if(m_classic_check_box->isChecked()) {
    tracks.push_back(Track::CLASSIC);
  }
  auto servers = std::vector<std::string>();
  if(m_live_trading_check_box->isChecked()) {
    servers.push_back("Live Trading");
  }
  if(m_staging_check_box->isChecked()) {
    servers.push_back("Staging");
  }
  if(m_uat_environment_check_box->isChecked()) {
    servers.push_back("UAT Environment");
  }
  if(m_local_environment_check_box->isChecked()) {
    servers.push_back("Local Environment");
  }
  m_window = new SignInWindow(SPIRE_VERSION, tracks, m_track, servers);
  m_window->connect_sign_in_signal(
    std::bind_front(&SignInUiTester::on_sign_in, this));
  m_window->connect_cancel_signal(
    std::bind_front(&SignInUiTester::on_cancel, this));
  m_window->installEventFilter(this);
  m_window->show();
}

void SignInUiTester::on_sign_in(const std::string& username,
    const std::string& password, Track track, const std::string& server) {
  m_signals_text->append(tr("SignIn(\"%1\", \"%2\", %3, \"%4\")\n").arg(
    QString::fromStdString(username), QString::fromStdString(password),
    to_text(track), QString::fromStdString(server)));
  m_accept_button->setEnabled(true);
  m_reject_button->setEnabled(true);
  m_error_button->setEnabled(true);
}

void SignInUiTester::on_cancel() {
  m_signals_text->append(tr("Cancel\n"));
  m_accept_button->setDisabled(true);
  m_reject_button->setDisabled(true);
  m_error_button->setDisabled(true);
}
