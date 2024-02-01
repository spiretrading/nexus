#include "Spire/LegacyUI/EnvironmentSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <QApplication>
#include <QMessageBox>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/Toolbar.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;
using namespace std;
using namespace std::filesystem;

bool Spire::LegacyUI::Export(const EnvironmentSettings& environmentSettings,
    const path& environmentPath) {
  ofstream writerStream;
  writerStream.open(environmentPath, ios::binary);
  if((writerStream.rdstate() & std::ifstream::failbit) != 0) {
    QMessageBox::warning(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to write to the specified path."));
    return false;
  }
  try {
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.SetSink(Ref(buffer));
    sender.Shuttle(environmentSettings);
    BasicOStreamWriter<ofstream*> writer(&writerStream);
    writer.Write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to export settings."));
    return false;
  }
  return true;
}

bool Spire::LegacyUI::Import(const path& environmentPath,
    EnvironmentSettings::TypeSet settings, bool apply,
    Out<UserProfile> userProfile) {
  ifstream readerStream;
  readerStream.open(environmentPath, ios::binary);
  if((readerStream.rdstate() & std::ifstream::failbit) != 0) {
    QMessageBox::warning(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to read from the specified path."));
    return false;
  }
  EnvironmentSettings environmentSettings;
  try {
    BasicIStreamReader<ifstream*> reader(&readerStream);
    SharedBuffer buffer;
    reader.Read(Store(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(Store(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(environmentSettings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to read from the specified path."));
    return false;
  }
  if(settings.Test(EnvironmentSettings::Type::WINDOW_LAYOUTS) &&
      environmentSettings.m_windowLayouts.is_initialized()) {
    for(auto widget : QApplication::topLevelWidgets()) {
      if(dynamic_cast<PersistentWindow*>(widget) != nullptr &&
          dynamic_cast<Toolbar*>(widget) == nullptr) {
        widget->close();
      }
    }
    for(auto& windowSettings : *environmentSettings.m_windowLayouts) {
      auto window = windowSettings->Reopen(Ref(*userProfile));
      if(dynamic_cast<PortfolioViewerWindow*>(window) != nullptr &&
          !userProfile->IsManager()) {
        continue;
      }
      window->show();
    }
  }
  if(settings.Test(EnvironmentSettings::Type::BOOK_VIEW) &&
      environmentSettings.m_bookViewProperties.is_initialized()) {
    userProfile->SetDefaultBookViewProperties(
      *environmentSettings.m_bookViewProperties);
  }
  if(settings.Test(EnvironmentSettings::Type::DASHBOARDS) &&
      environmentSettings.m_dashboards.is_initialized()) {
    userProfile->GetSavedDashboards() = *environmentSettings.m_dashboards;
  }
  if(settings.Test(EnvironmentSettings::Type::ORDER_IMBALANCE_INDICATOR) &&
      environmentSettings.m_orderImbalanceIndicatorProperties.
      is_initialized()) {
    userProfile->SetDefaultOrderImbalanceIndicatorProperties(
      *environmentSettings.m_orderImbalanceIndicatorProperties);
  }
  if(settings.Test(EnvironmentSettings::Type::INTERACTIONS) &&
      environmentSettings.m_interactionsProperties.is_initialized()) {
    userProfile->GetInteractionProperties() =
      *environmentSettings.m_interactionsProperties;
  }
  if(settings.Test(EnvironmentSettings::Type::KEY_BINDINGS) &&
      environmentSettings.m_keyBindings.is_initialized()) {
    userProfile->SetKeyBindings(*environmentSettings.m_keyBindings);
  }
  if(settings.Test(EnvironmentSettings::Type::PORTFOLIO_VIEWER) &&
      environmentSettings.m_portfolioViewerProperties.is_initialized()) {
    userProfile->SetDefaultPortfolioViewerProperties(
      *environmentSettings.m_portfolioViewerProperties);
  }
  if(settings.Test(EnvironmentSettings::Type::TIME_AND_SALES) &&
      environmentSettings.m_timeAndSalesProperties.is_initialized()) {
    userProfile->SetDefaultTimeAndSalesProperties(
      *environmentSettings.m_timeAndSalesProperties);
  }
  if(!apply) {
    return true;
  }
  for(auto widget : QApplication::topLevelWidgets()) {
    if(auto bookView = dynamic_cast<BookViewWindow*>(widget)) {
      if(environmentSettings.m_bookViewProperties.is_initialized()) {
        bookView->SetProperties(*environmentSettings.m_bookViewProperties);
      }
    } else if(auto orderImbalanceIndicator =
        dynamic_cast<OrderImbalanceIndicatorWindow*>(widget)) {
      if(environmentSettings.m_orderImbalanceIndicatorProperties.
          is_initialized()) {
        auto model = std::make_shared<OrderImbalanceIndicatorModel>(
          Ref(*userProfile),
          *environmentSettings.m_orderImbalanceIndicatorProperties);
        orderImbalanceIndicator->SetModel(model);
      }
    } else if(auto portfolioViewer =
        dynamic_cast<PortfolioViewerWindow*>(widget)) {
      if(environmentSettings.m_portfolioViewerProperties.is_initialized()) {
        portfolioViewer->SetProperties(
          *environmentSettings.m_portfolioViewerProperties);
      }
    } else if(auto timeAndSales = dynamic_cast<TimeAndSalesWindow*>(widget)) {
      if(environmentSettings.m_timeAndSalesProperties.is_initialized()) {
        timeAndSales->SetProperties(
          *environmentSettings.m_timeAndSalesProperties);
      }
    }
  }
  return true;
}
