#include "Spire/RiskTimer/RiskTimerProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;

namespace {
  const auto RISK_TIMER_DIALOG_WIDTH = 309;

  void LoadDefault(Out<UserProfile> userProfile) {
    userProfile->GetRiskTimerProperties() = RiskTimerProperties::GetDefault();
  }
}

RiskTimerProperties RiskTimerProperties::GetDefault() {
  auto properties = RiskTimerProperties();
  auto availableGeometry =
    QGuiApplication::primaryScreen()->availableGeometry();
  auto riskTimerDialogInitialPosition = QPoint((availableGeometry.width() -
    RISK_TIMER_DIALOG_WIDTH) / 2, (14 * availableGeometry.height()) / 100);
  properties.SetRiskTimerDialogInitialPosition(riskTimerDialogInitialPosition);
  return properties;
}

void RiskTimerProperties::Load(Out<UserProfile> userProfile) {
  auto filePath = userProfile->GetProfilePath() / "risk_timer.dat";
  if(!std::filesystem::exists(filePath)) {
    LoadDefault(Store(userProfile));
    return;
  }
  auto properties = RiskTimerProperties();
  try {
    auto reader =
      BasicIStreamReader<std::ifstream>(Initialize(filePath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
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
  auto filePath = userProfile.GetProfilePath() / "risk_timer.dat";
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(userProfile.GetRiskTimerProperties());
    auto writer =
      BasicOStreamWriter<std::ofstream>(Initialize(filePath, std::ios::binary));
    writer.Write(buffer);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save time and sales properties."));
  }
}

const QPoint& RiskTimerProperties::GetRiskTimerDialogInitialPosition() const {
  return m_riskTimerDialogInitialPosition;
}

void RiskTimerProperties::SetRiskTimerDialogInitialPosition(
    const QPoint& position) {
  m_riskTimerDialogInitialPosition = position;
}
