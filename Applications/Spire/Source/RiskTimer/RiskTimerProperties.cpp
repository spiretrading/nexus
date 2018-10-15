#include "Spire/RiskTimer/RiskTimerProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMessageBox>
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Threading;
using namespace boost;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

namespace {
  const int RISK_TIMER_DIALOG_WIDTH = 309;

  void LoadDefault(Out<UserProfile> userProfile) {
    userProfile->GetRiskTimerProperties() = RiskTimerProperties::GetDefault();
  }
}

RiskTimerProperties RiskTimerProperties::GetDefault() {
  RiskTimerProperties properties;
  QPoint riskTimerDialogInitialPosition(
    (QApplication::desktop()->availableGeometry().width() -
    RISK_TIMER_DIALOG_WIDTH) / 2,
    (14 * QApplication::desktop()->availableGeometry().height()) / 100);
  properties.SetRiskTimerDialogInitialPosition(riskTimerDialogInitialPosition);
  return properties;
}

void RiskTimerProperties::Load(Out<UserProfile> userProfile) {
  path filePath = userProfile->GetProfilePath() / "risk_timer.dat";
  if(!exists(filePath)) {
    LoadDefault(Store(userProfile));
    return;
  }
  RiskTimerProperties properties;
  try {
    BasicIStreamReader<ifstream> reader(Initialize(filePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(properties);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load risk timer settings, using defaults."));
    LoadDefault(Store(userProfile));
    return;
  }
  userProfile->GetRiskTimerProperties() = properties;
}

void RiskTimerProperties::Save(const UserProfile& userProfile) {
  path filePath = userProfile.GetProfilePath() / "risk_timer.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(userProfile.GetRiskTimerProperties());
    BasicOStreamWriter<ofstream> writer(Initialize(filePath, ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save time and sales properties."));
  }
}

RiskTimerProperties::RiskTimerProperties() {}

const QPoint& RiskTimerProperties::GetRiskTimerDialogInitialPosition() const {
  return m_riskTimerDialogInitialPosition;
}

void RiskTimerProperties::SetRiskTimerDialogInitialPosition(
    const QPoint& position) {
  m_riskTimerDialogInitialPosition = position;
}
