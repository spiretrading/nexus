#include "Spire/UI/EnvironmentSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <QApplication>
#include <QMessageBox>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/UI/PersistentWindow.hpp"
#include "Spire/UI/Toolbar.hpp"
#include "Spire/UI/UISerialization.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::UI;
using namespace std;
using namespace std::filesystem;

bool Spire::UI::Export(const EnvironmentSettings& environmentSettings,
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
    RegisterSpireTypes(out(typeRegistry));
    auto sender = BinarySender<SharedBuffer>(Ref(typeRegistry));
    SharedBuffer buffer;
    sender.set(Ref(buffer));
    sender.shuttle(environmentSettings);
    BasicOStreamWriter<ofstream*> writer(&writerStream);
    writer.write(buffer);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Warning"),
      QObject::tr("Unable to export settings."));
    return false;
  }
  return true;
}

bool Spire::UI::Import(const path& environmentPath,
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
    reader.read(out(buffer));
    TypeRegistry<BinarySender<SharedBuffer>> typeRegistry;
    RegisterSpireTypes(out(typeRegistry));
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(typeRegistry));
    receiver.set(Ref(buffer));
    receiver.shuttle(environmentSettings);
  } catch(std::exception&) {
    QMessageBox::warning(nullptr, QObject::tr("Error"),
      QObject::tr("Unable to read from the specified path."));
    return false;
  }
  if(settings.test(EnvironmentSettings::Type::WINDOW_LAYOUTS) &&
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
  if(settings.test(EnvironmentSettings::Type::BOOK_VIEW) &&
      environmentSettings.m_bookViewProperties.is_initialized()) {
    userProfile->SetDefaultBookViewProperties(
      *environmentSettings.m_bookViewProperties);
  }
  if(settings.test(EnvironmentSettings::Type::DASHBOARDS) &&
      environmentSettings.m_dashboards.is_initialized()) {
    userProfile->GetSavedDashboards() = *environmentSettings.m_dashboards;
  }
  if(settings.test(EnvironmentSettings::Type::ORDER_IMBALANCE_INDICATOR) &&
      environmentSettings.m_orderImbalanceIndicatorProperties.
      is_initialized()) {
    userProfile->SetDefaultOrderImbalanceIndicatorProperties(
      *environmentSettings.m_orderImbalanceIndicatorProperties);
  }
  if(settings.test(EnvironmentSettings::Type::INTERACTIONS) &&
      environmentSettings.m_interactionsProperties.is_initialized()) {
    userProfile->GetInteractionProperties() =
      *environmentSettings.m_interactionsProperties;
  }
  if(settings.test(EnvironmentSettings::Type::KEY_BINDINGS) &&
      environmentSettings.m_keyBindings.is_initialized()) {
    userProfile->SetKeyBindings(*environmentSettings.m_keyBindings);
  }
  if(settings.test(EnvironmentSettings::Type::PORTFOLIO_VIEWER) &&
      environmentSettings.m_portfolioViewerProperties.is_initialized()) {
    userProfile->SetDefaultPortfolioViewerProperties(
      *environmentSettings.m_portfolioViewerProperties);
  }
  if(settings.test(EnvironmentSettings::Type::TIME_AND_SALES) &&
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
