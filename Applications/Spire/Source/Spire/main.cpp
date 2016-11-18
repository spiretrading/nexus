#include <Beam/Network/IpAddress.hpp>
#include <Beam/Network/SocketThreadPool.hpp>
#include <Beam/ServiceLocator/ApplicationDefinitions.hpp>
#include <Beam/Threading/TimerThreadPool.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtPlugin>
#include "Spire/Login/LoginDialog.hpp"
#include "Spire/Spire/ServiceClients.hpp"
#include "Spire/Version.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Threading;
using namespace boost;
using namespace boost::filesystem;
using namespace Spire;
using namespace std;

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

inline void InitializeResources() {
  Q_INIT_RESOURCE(Resources);
}

namespace {
  vector<LoginDialog::ServerInstance> ParseServerInstances(
    const YAML::Node& config) {
    vector<LoginDialog::ServerInstance> serverInstances;
    try {
      auto serverInstancesNode = config.FindValue("servers");
      if (serverInstancesNode == nullptr) {
        QMessageBox::critical(nullptr, QObject::tr("Error"),
          QObject::tr("Invalid configuration file."));
        return{};
      }
      for (auto& node : *serverInstancesNode) {
        auto name = Extract<string>(node, "name");
        auto address = Extract<IpAddress>(node, "address");
        serverInstances.emplace_back(name, address);
      }
    }
    catch (const std::exception&) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Invalid configuration file."));
      return{};
    }
    return serverInstances;
  }
}

int main(int argc, char** argv){
#ifdef _DEBUG
  freopen("stdout.log", "w", stdout);
  freopen("stderr.log", "w", stderr);
#endif
  QApplication application{ argc, argv };
  application.setOrganizationName(QObject::tr("Eidolon Systems"));
  application.setApplicationName(QObject::tr("Spire"));
  application.setApplicationVersion(SPIRE_VERSION);
  application.setStyleSheet(
    "QToolTip {"
    "  background: red;"
    "}");
  InitializeResources();

  auto applicationPath = QStandardPaths::writableLocation(
    QStandardPaths::DataLocation);
  path configPath = applicationPath.toStdString();
  if (!exists(configPath)) {
    create_directories(configPath);
  }
  configPath /= "config.yml";
  if (!is_regular(configPath)) {
    filesystem::ofstream configFile{ configPath };
    configFile <<
      "---\n"
      "servers:\n"
      "  - name: \"primary\"\n"
      "    address: 192.168.17.129:20000\n"
      "...\n";
  }
  YAML::Node config;
  try {
    filesystem::ifstream configStream{ configPath };
    if (!configStream.good()) {
      QMessageBox::critical(nullptr, QObject::tr("Error"),
        QObject::tr("Unable to load configuration: config.yml"));
      return -1;
    }
    YAML::Parser configParser{ configStream };
    configParser.GetNextDocument(config);
  }
  catch (YAML::ParserException&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Invalid configuration file."));
    return -1;
  }
  auto serverInstances = ParseServerInstances(config);
  if (serverInstances.empty()) {
    return -1;
  }
  SocketThreadPool socketThreadPool;
  TimerThreadPool timerThreadPool;

  LoginDialog loginDialog{ std::move(serverInstances), Ref(socketThreadPool),
    Ref(timerThreadPool) };
  auto loginResultCode = loginDialog.exec();
  if (loginResultCode == QDialog::Rejected) {
    return -1;
  }
  auto serviceClients = loginDialog.GetServiceClients();
  try {
    //    userProfile.CreateProfilePath();
  }
  catch (const std::exception&) {
    QMessageBox::critical(nullptr, QObject::tr("Error"),
      QObject::tr("Error creating profile path."));
    return -1;
  }
  return application.exec();
}
