#include "Spire/Toolbar/UserSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <QApplication>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Canvas/OrderExecutionNodes/TickerPortfolioNode.hpp"
#include "Spire/Canvas/Types/TickerType.hpp"
#include "Spire/Canvas/ValueNodes/TickerNode.hpp"
#include "Spire/KeyBindings/LegacyKeyBindings.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"
#include "Spire/LegacyUI/UISerialization.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorModel.hpp"
#include "Spire/OrderImbalanceIndicator/OrderImbalanceIndicatorWindow.hpp"
#include "Spire/PortfolioViewer/PortfolioViewerWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  struct LegacyEnvironmentSettings {
    boost::optional<BookViewProperties> m_book_view_properties;
    boost::optional<SavedDashboards> m_dashboards;
    boost::optional<OrderImbalanceIndicatorProperties>
      m_order_imbalance_indicator_properties;
    boost::optional<Nexus::ScopeMap<LegacyInteractionsProperties>>
      m_interactions_properties;
    boost::optional<LegacyKeyBindings> m_key_bindings;
    boost::optional<PortfolioViewerProperties> m_portfolio_viewer_properties;
    boost::optional<TimeAndSalesProperties> m_time_and_sales_properties;
    boost::optional<std::vector<std::shared_ptr<WindowSettings>>>
      m_window_layouts;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("book_view_properties", m_book_view_properties);
      shuttle.shuttle("dashboards", m_dashboards);
      shuttle.shuttle("order_imbalance_indicator_properties",
        m_order_imbalance_indicator_properties);
      shuttle.shuttle("interactions_properties", m_interactions_properties);
      shuttle.shuttle("key_bindings", m_key_bindings);
      shuttle.shuttle("portfolio_viewer_properties",
        m_portfolio_viewer_properties);
      shuttle.shuttle("time_and_sales_properties", m_time_and_sales_properties);
      shuttle.shuttle("window_layouts", m_window_layouts);
    }
  };

  void read_legacy_settings(const std::filesystem::path& path,
      UserSettings::Categories categories, Out<UserSettings> settings) {
    auto reader =
      BasicIStreamReader<std::ifstream>(init(path, std::ios::binary));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto legacy = LegacyEnvironmentSettings();
    auto load = [&] (TypeRegistry<BinarySender<SharedBuffer>>& registry) {
      auto receiver = BinaryReceiver<SharedBuffer>(Ref(registry));
      receiver.set(Ref(buffer));
      auto loaded = LegacyEnvironmentSettings();
      receiver.shuttle(loaded);
      legacy = std::move(loaded);
    };
    auto current_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    RegisterSpireTypes(out(current_registry));
    try {
      load(current_registry);
    } catch(const std::exception&) {
      auto legacy_registry = TypeRegistry<BinarySender<SharedBuffer>>();
      legacy_registry.add<TickerNode>("Spire.SecurityNode");
      legacy_registry.add<TickerType>("Spire.SecurityType");
      legacy_registry.add<TickerPortfolioNode>("Spire.SecurityPortfolioNode");
      RegisterSpireTypes(out(legacy_registry));
      load(legacy_registry);
    }
    settings->m_book_view_properties = legacy.m_book_view_properties;
    settings->m_dashboards = legacy.m_dashboards;
    settings->m_order_imbalance_indicator_properties =
      legacy.m_order_imbalance_indicator_properties;
    settings->m_portfolio_properties = legacy.m_portfolio_viewer_properties;
    settings->m_time_and_sales_properties = legacy.m_time_and_sales_properties;
    settings->m_layouts = legacy.m_window_layouts;
    if(categories.test(UserSettings::Category::KEY_BINDINGS) &&
        legacy.m_key_bindings) {
      auto interactions = legacy.m_interactions_properties.value_or(
        Nexus::ScopeMap<LegacyInteractionsProperties>(
          "Global", LegacyInteractionsProperties()));
      apply_legacy_key_bindings(
        *legacy.m_key_bindings, interactions, *settings->m_key_bindings);
    }
  }
}

void Spire::export_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, const UserProfile& user_profile) {
  auto settings = UserSettings();
  if(categories.test(UserSettings::Category::BOOK_VIEW)) {
    settings.m_book_view_properties =
      user_profile.GetDefaultBookViewProperties();
  }
  if(categories.test(UserSettings::Category::WATCHLIST)) {
    settings.m_dashboards = user_profile.GetSavedDashboards();
  }
  if(categories.test(UserSettings::Category::ORDER_IMBALANCE_INDICATOR)) {
    settings.m_order_imbalance_indicator_properties =
      user_profile.GetDefaultOrderImbalanceIndicatorProperties();
  }
  if(categories.test(UserSettings::Category::KEY_BINDINGS)) {
    settings.m_key_bindings = user_profile.GetKeyBindings();
  }
  if(categories.test(UserSettings::Category::PORTFOLIO)) {
    settings.m_portfolio_properties =
      user_profile.GetDefaultPortfolioViewerProperties();
  }
  if(categories.test(UserSettings::Category::TIME_AND_SALES)) {
    settings.m_time_and_sales_properties =
      user_profile.GetDefaultTimeAndSalesProperties();
  }
  if(categories.test(UserSettings::Category::LAYOUT)) {
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
    RegisterSpireTypes(out(registry));
    auto sender = JsonSender<SharedBuffer>(Ref(registry));
    auto buffer = SharedBuffer();
    sender.set(Ref(buffer));
    sender.shuttle(settings);
    auto writer = BasicOStreamWriter<std::ofstream>(init(path));
    writer.write(buffer);
  } catch(const std::exception&) {
    throw std::runtime_error(
      QObject::tr("Unable to write to the specified path.").toStdString());
  }
}

void Spire::import_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, Out<UserProfile> user_profile) {
  auto settings = UserSettings();
  if(categories.test(UserSettings::Category::KEY_BINDINGS)) {
    settings.m_key_bindings = user_profile->GetKeyBindings();
  } else {
    settings.m_key_bindings = std::make_shared<KeyBindingsModel>();
  }
  try {
    if(path.extension() == ".sps") {
      read_legacy_settings(path, categories, out(settings));
    } else {
      auto reader = BasicIStreamReader<std::ifstream>(init(path));
      auto buffer = SharedBuffer();
      reader.read(out(buffer));
      auto registry = TypeRegistry<JsonSender<SharedBuffer>>();
      RegisterSpireTypes(out(registry));
      auto receiver = JsonReceiver<SharedBuffer>(Ref(registry));
      receiver.set(Ref(buffer));
      receiver.shuttle(settings);
    }
  } catch(const std::exception&) {
    throw std::runtime_error(
      QObject::tr("Unable to read from the specified path.").toStdString());
  }
  if(categories.test(UserSettings::Category::LAYOUT) && settings.m_layouts) {
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
  if(categories.test(UserSettings::Category::BOOK_VIEW) &&
      settings.m_book_view_properties) {
    user_profile->SetDefaultBookViewProperties(
      *settings.m_book_view_properties);
  }
  if(categories.test(UserSettings::Category::WATCHLIST) &&
      settings.m_dashboards) {
    user_profile->GetSavedDashboards() = *settings.m_dashboards;
  }
  if(categories.test(UserSettings::Category::ORDER_IMBALANCE_INDICATOR) &&
      settings.m_order_imbalance_indicator_properties) {
    user_profile->SetDefaultOrderImbalanceIndicatorProperties(
      *settings.m_order_imbalance_indicator_properties);
  }
  if(categories.test(UserSettings::Category::PORTFOLIO) &&
      settings.m_portfolio_properties) {
    user_profile->SetDefaultPortfolioViewerProperties(
      *settings.m_portfolio_properties);
  }
  if(categories.test(UserSettings::Category::TIME_AND_SALES) &&
      settings.m_time_and_sales_properties) {
    user_profile->SetDefaultTimeAndSalesProperties(
      *settings.m_time_and_sales_properties);
  }
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
    } else if(auto time_and_sales = dynamic_cast<TimeAndSalesWindow*>(widget)) {
      if(settings.m_time_and_sales_properties) {
        time_and_sales->SetProperties(*settings.m_time_and_sales_properties);
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
