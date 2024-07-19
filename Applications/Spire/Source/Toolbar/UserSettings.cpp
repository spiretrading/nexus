#include "Spire/Toolbar/UserSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <QApplication>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

void Spire::export_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, const UserProfile& user_profile) {
  auto settings = UserSettings();
  if(categories.Test(UserSettings::Category::BOOK_VIEW)) {
    settings.m_book_view_properties =
      user_profile.GetDefaultBookViewProperties();
  }
  if(categories.Test(UserSettings::Category::WATCHLIST)) {
    settings.m_dashboards = user_profile.GetSavedDashboards();
  }
  if(categories.Test(UserSettings::Category::ORDER_IMBALANCE_INDICATOR)) {
    settings.m_order_imbalance_indicator_properties =
      user_profile.GetDefaultOrderImbalanceIndicatorProperties();
  }
  if(categories.Test(UserSettings::Category::KEY_BINDINGS)) {
    settings.m_key_bindings = user_profile.GetKeyBindings();
  }
  if(categories.Test(UserSettings::Category::PORTFOLIO)) {
    settings.m_portfolio_properties =
      user_profile.GetDefaultPortfolioViewerProperties();
  }
/** TODO
  if(categories.Test(UserSettings::Category::TIME_AND_SALES)) {
    settings.m_time_and_sales_properties =
      user_profile.GetDefaultTimeAndSalesProperties();
  }
*/
  if(categories.Test(UserSettings::Category::LAYOUT)) {
    auto layouts = std::vector<std::shared_ptr<WindowSettings>>();
    for(auto& widget : QApplication::topLevelWidgets()) {
      auto window = dynamic_cast<PersistentWindow*>(widget);
      if(window && !dynamic_cast<ToolbarWindow*>(widget)) {
        layouts.push_back(window->GetWindowSettings());
      }
    }
    settings.m_layouts = std::move(layouts);
  }
  try {
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    auto buffer = SharedBuffer();
    sender.SetSink(Ref(buffer));
    sender.Shuttle(settings);
    auto writer = BasicOStreamWriter<std::ofstream>(Initialize(path));
    writer.Write(buffer);
  } catch(const std::exception&) {
    throw std::runtime_error(
      QObject::tr("Unable to write to the specified path.").toStdString());
  }
}

void Spire::import_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, Out<UserProfile> user_profile) {
  auto settings = UserSettings();
  if(categories.Test(UserSettings::Category::KEY_BINDINGS) &&
      settings.m_key_bindings) {
    settings.m_key_bindings = user_profile->GetKeyBindings();
  } else {
    settings.m_key_bindings =
      std::make_shared<KeyBindingsModel>(user_profile->GetMarketDatabase());;
  }
  try {
    auto reader = BasicIStreamReader<std::ifstream>(Initialize(path));
    auto buffer = SharedBuffer();
    reader.Read(Store(buffer));
    auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(Store(registry));
    auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
    receiver.SetSource(Ref(buffer));
    receiver.Shuttle(settings);
  } catch(const std::exception&) {
    throw std::runtime_error(
      QObject::tr("Unable to read from the specified path.").toStdString());
  }
  if(categories.Test(UserSettings::Category::LAYOUT) && settings.m_layouts) {
    for(auto& widget : QApplication::topLevelWidgets()) {
      if(dynamic_cast<PersistentWindow*>(widget) &&
          !dynamic_cast<ToolbarWindow*>(widget)) {
        widget->close();
      }
    }
    for(auto& layout : *settings.m_layouts) {
      auto window = layout->Reopen(Ref(*user_profile));
      if(dynamic_cast<PortfolioViewerWindow*>(window) &&
          !user_profile->IsManager()) {
        continue;
      }
      window->show();
    }
  }
  if(categories.Test(UserSettings::Category::BOOK_VIEW) &&
      settings.m_book_view_properties) {
    user_profile->SetDefaultBookViewProperties(
      *settings.m_book_view_properties);
  }
  if(categories.Test(UserSettings::Category::WATCHLIST) &&
      settings.m_dashboards) {
    user_profile->GetSavedDashboards() = *settings.m_dashboards;
  }
  if(categories.Test(UserSettings::Category::ORDER_IMBALANCE_INDICATOR) &&
      settings.m_order_imbalance_indicator_properties) {
    user_profile->SetDefaultOrderImbalanceIndicatorProperties(
      *settings.m_order_imbalance_indicator_properties);
  }
  if(categories.Test(UserSettings::Category::PORTFOLIO) &&
      settings.m_portfolio_properties) {
    user_profile->SetDefaultPortfolioViewerProperties(
      *settings.m_portfolio_properties);
  }
/** TODO
  if(categories.Test(UserSettings::Category::TIME_AND_SALES) &&
      settings.m_time_and_sales_properties) {
    user_profile->SetDefaultTimeAndSalesProperties(
      *settings.m_time_and_sales_properties);
  }
*/
  for(auto widget : QApplication::topLevelWidgets()) {
    if(auto book_view = dynamic_cast<BookViewWindow*>(widget)) {
      if(settings.m_book_view_properties) {
        book_view->SetProperties(*settings.m_book_view_properties);
      }
    } else if(auto order_imbalance_indicator =
        dynamic_cast<OrderImbalanceIndicatorWindow*>(widget)) {
      if(settings.m_order_imbalance_indicator_properties) {
        auto model = std::make_shared<OrderImbalanceIndicatorModel>(
          Ref(*user_profile), *settings.m_order_imbalance_indicator_properties);
        order_imbalance_indicator->SetModel(model);
      }
    } else if(auto portfolio = dynamic_cast<PortfolioViewerWindow*>(widget)) {
      if(settings.m_portfolio_properties) {
        portfolio->SetProperties(*settings.m_portfolio_properties);
      }
    }
  }
}

const QString& Spire::to_text(UserSettings::Category category) {
  if(category == UserSettings::Category::BOOK_VIEW) {
    static const auto value = QObject::tr("Book View");
    return value;
  } else if(category == UserSettings::Category::ORDER_IMBALANCE_INDICATOR) {
    static const auto value = QObject::tr("Order Imbalance Indicator");
    return value;
  } else if(category == UserSettings::Category::INTERACTIONS) {
    static const auto value = QObject::tr("Interactions");
    return value;
  } else if(category == UserSettings::Category::KEY_BINDINGS) {
    static const auto value = QObject::tr("Key Bindings");
    return value;
  } else if(category == UserSettings::Category::PORTFOLIO) {
    static const auto value = QObject::tr("Portfolio");
    return value;
  } else if(category == UserSettings::Category::TIME_AND_SALES) {
    static const auto value = QObject::tr("Time and Sales");
    return value;
  } else if(category == UserSettings::Category::WATCHLIST) {
    static const auto value = QObject::tr("Watchlist");
    return value;
  } else if(category == UserSettings::Category::LAYOUT) {
    static const auto value = QObject::tr("Layout");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}
