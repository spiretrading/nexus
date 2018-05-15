#include "Spire/KeyBindings/InteractionsProperties.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <QMessageBox>
#include "Nexus/Definitions/Country.hpp"
#include "Spire/Spire/UserProfile.hpp"
#include "Spire/UI/UISerialization.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace boost::filesystem;
using namespace Nexus;
using namespace Spire;
using namespace std;

InteractionsProperties InteractionsProperties::GetDefaultProperties() {
  InteractionsProperties properties;
  properties.m_defaultQuantity = 100;
  properties.m_quantityIncrements.fill(100);
  properties.m_priceIncrements.fill(Money::CENT);
  properties.m_cancelOnFill = false;
  return properties;
}

RegionMap<InteractionsProperties> InteractionsProperties::
    GetDefaultPropertiesMap(const UserProfile& userProfile) {
  RegionMap<InteractionsProperties> propertiesMap("Global",
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
    BasicIStreamReader<boost::filesystem::ifstream> reader(
      Initialize(interactionsFilePath, ios::binary));
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    BinaryReceiver<SharedBuffer> receiver(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(userProfile->GetInteractionProperties());
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
    RegisterSpireTypes(Store(typeRegistry));
    BinarySender<SharedBuffer> sender(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(userProfile.GetInteractionProperties());
    BasicOStreamWriter<boost::filesystem::ofstream> writer(
      Initialize(keyBindingsFilePath, ios::binary));
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to save key bindings."));
  }
}
