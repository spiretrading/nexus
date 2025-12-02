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
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

using namespace Beam;
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
    LoadDefault(out(userProfile));
    return;
  }
  auto properties = RiskTimerProperties();
  try {
    auto reader =
      BasicIStreamReader<std::ifstream>(init(filePath, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(properties);
  } catch(const std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load risk timer settings, using defaults."));
    LoadDefault(out(userProfile));
    return;
  }
  userProfile->GetRiskTimerProperties() = properties;
}

void RiskTimerProperties::Save(const UserProfile& userProfile) {
  auto filePath = userProfile.GetProfilePath() / "risk_timer.dat";
  try {
    auto typeRegistry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.shuttle(userProfile.GetRiskTimerProperties());
    auto writer =
      BasicOStreamWriter<std::ofstream>(init(filePath, std::ios::binary));
    writer.write(buffer);
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
