#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include <filesystem>
#include <fstream>
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <QMessageBox>
#include "Nexus/Definitions/Country.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;
using namespace std::filesystem;

InteractionsProperties InteractionsProperties::GetDefaultProperties() {
  InteractionsProperties properties;
  properties.m_defaultQuantity = 100;
  properties.m_quantityIncrements.fill(100);
  properties.m_priceIncrements.fill(Money::CENT);
  properties.m_cancelOnFill = false;
  return properties;
}

ScopeTable<InteractionsProperties> InteractionsProperties::
    GetDefaultPropertiesMap(const UserProfile& userProfile) {
  ScopeTable<InteractionsProperties> propertiesMap("Global",
    GetDefaultProperties());
  return propertiesMap;
}

void InteractionsProperties::Load(Out<UserProfile> userProfile) {
  path interactionsFilePath = userProfile->GetProfilePath() /
    "interactions.dat";
  if(!exists(interactionsFilePath)) {
    userProfile->GetInteractionProperties() = GetDefaultPropertiesMap(
      *userProfile);
    return;
  }
  try {
    BasicIStreamReader<ifstream> reader(
      init(interactionsFilePath, ios::binary));
    SharedBuffer buffer;
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(userProfile->GetInteractionProperties());
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to load key bindings, using defaults."));
    userProfile->GetInteractionProperties() = GetDefaultPropertiesMap(
      *userProfile);
  }
}

void InteractionsProperties::Save(const UserProfile& userProfile) {
  path keyBindingsFilePath = userProfile.GetProfilePath() / "interactions.dat";
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    sender.shuttle(userProfile.GetInteractionProperties());
    BasicOStreamWriter<ofstream> writer(
      init(keyBindingsFilePath, ios::binary));
    writer.write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save key bindings."));
  }
}
