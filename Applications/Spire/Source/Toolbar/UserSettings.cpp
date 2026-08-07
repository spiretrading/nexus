#include "Spire/Toolbar/UserSettings.hpp"
#include <Beam/IO/BasicIStreamReader.hpp>
#include <Beam/IO/BasicOStreamWriter.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/BinaryReceiver.hpp>
#include <Beam/Serialization/BinarySender.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/Utilities/AssertionException.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <QApplication>
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/BookView/LegacyBookViewWindowSettings.hpp"
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
#include "Spire/TimeAndSales/LegacyTimeAndSalesWindowSettings.hpp"
#include "Spire/Toolbar/ToolbarWindow.hpp"

using namespace Beam;
using namespace boost;
using namespace Spire;
using namespace Spire::LegacyUI;

namespace {
  struct LegacyEnvironmentSettings {
    optional<LegacyBookViewWindowSettings::Properties> m_book_view_properties;
    optional<SavedDashboards> m_dashboards;
    optional<OrderImbalanceIndicatorProperties>
      m_order_imbalance_indicator_properties;
    optional<Nexus::ScopeMap<LegacyInteractionsProperties>>
      m_interactions_properties;
    optional<LegacyKeyBindings> m_key_bindings;
    optional<PortfolioViewerProperties> m_portfolio_viewer_properties;
    optional<LegacyTimeAndSalesWindowSettings::Properties>
      m_time_and_sales_properties;
    optional<std::vector<std::shared_ptr<WindowSettings>>>
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
    if(legacy.m_book_view_properties) {
      settings->m_book_view_properties =
        to_book_view_properties(*legacy.m_book_view_properties);
    }
    settings->m_dashboards = legacy.m_dashboards;
    settings->m_order_imbalance_indicator_properties =
      legacy.m_order_imbalance_indicator_properties;
    settings->m_portfolio_properties = legacy.m_portfolio_viewer_properties;
    if(legacy.m_time_and_sales_properties) {
      settings->m_time_and_sales_properties =
        to_time_and_sales_properties(*legacy.m_time_and_sales_properties);
    }
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

  struct LegacyUserSettings {
    optional<LegacyBookViewWindowSettings::Properties> m_book_view_properties;
    optional<SavedDashboards> m_dashboards;
    optional<OrderImbalanceIndicatorProperties>
      m_order_imbalance_indicator_properties;
    std::shared_ptr<KeyBindingsModel> m_key_bindings;
    optional<PortfolioViewerProperties> m_portfolio_properties;
    optional<LegacyTimeAndSalesWindowSettings::Properties>
      m_time_and_sales_properties;
    optional<std::vector<std::shared_ptr<WindowSettings>>> m_layouts;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("book_view_properties", m_book_view_properties);
      shuttle.shuttle("dashboards", m_dashboards);
      shuttle.shuttle("order_imbalance_indicator_properties",
        m_order_imbalance_indicator_properties);
      shuttle.shuttle("key_bindings", *m_key_bindings);
      shuttle.shuttle("portfolio_viewer_properties", m_portfolio_properties);
      shuttle.shuttle("time_and_sales_properties", m_time_and_sales_properties);
      shuttle.shuttle("window_layouts", m_layouts);
    }
  };

  void read_json_settings(
      const std::filesystem::path& path, Out<UserSettings> settings) {
    auto reader = BasicIStreamReader<std::ifstream>(init(path));
    auto buffer = SharedBuffer();
    reader.read(out(buffer));
    auto current_registry = TypeRegistry<JsonSender<SharedBuffer>>();
    RegisterSpireTypes(out(current_registry));
    try {
      auto receiver = JsonReceiver<SharedBuffer>(Ref(current_registry));
      receiver.set(Ref(buffer));
      receiver.shuttle(*settings);
      return;
    } catch(const std::exception&) {}
    auto legacy_registry = TypeRegistry<JsonSender<SharedBuffer>>();
    legacy_registry.add<TickerNode>("Spire.SecurityNode");
    legacy_registry.add<TickerType>("Spire.SecurityType");
    legacy_registry.add<TickerPortfolioNode>("Spire.SecurityPortfolioNode");
    RegisterSpireTypes(out(legacy_registry));
    auto text = std::string(buffer.get_data(), buffer.get_size());
    replace_all(text, "\"region\":", "\"scope\":");
    replace_all(text, "\"securities\":", "\"tickers\":");
    replace_all(text, "\"security_view_stack\":", "\"ticker_view_stack\":");
    replace_all(text, "\"security_view\":", "\"ticker_view\":");
    replace_all(text, "\"security\":", "\"ticker\":");
    auto legacy = LegacyUserSettings();
    legacy.m_key_bindings = settings->m_key_bindings;
    auto legacy_buffer = SharedBuffer(text.data(), text.size());
    auto receiver = JsonReceiver<SharedBuffer>(Ref(legacy_registry));
    receiver.set(Ref(legacy_buffer));
    receiver.shuttle(legacy);
    if(legacy.m_book_view_properties) {
      settings->m_book_view_properties =
        to_book_view_properties(*legacy.m_book_view_properties);
    }
    settings->m_dashboards = legacy.m_dashboards;
    settings->m_order_imbalance_indicator_properties =
      legacy.m_order_imbalance_indicator_properties;
    settings->m_portfolio_properties = legacy.m_portfolio_properties;
    if(legacy.m_time_and_sales_properties) {
      settings->m_time_and_sales_properties =
        to_time_and_sales_properties(*legacy.m_time_and_sales_properties);
    }
    settings->m_layouts = legacy.m_layouts;
  }
}

void Spire::export_settings(UserSettings::Categories categories,
    const std::filesystem::path& path, const UserProfile& user_profile) {
  auto settings = UserSettings();
  if(categories.test(UserSettings::Category::BOOK_VIEW)) {
    settings.m_book_view_properties =
      user_profile.GetBookViewPropertiesWindowFactory()->
        get_properties()->get();
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
      user_profile.GetTimeAndSalesPropertiesWindowFactory()->
        get_properties()->get();
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

std::vector<QWidget*> Spire::import_settings(
    UserSettings::Categories categories, const std::filesystem::path& path,
    Out<UserProfile> user_profile) {
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
      read_json_settings(path, out(settings));
    }
  } catch(const std::exception&) {
    throw std::runtime_error(
      QObject::tr("Unable to read from the specified path.").toStdString());
  }
  auto windows = std::vector<QWidget*>();
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
      windows.push_back(window);
    }
  }
  if(categories.test(UserSettings::Category::BOOK_VIEW) &&
      settings.m_book_view_properties) {
    user_profile->GetBookViewPropertiesWindowFactory()->get_properties()->set(
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
    user_profile->GetTimeAndSalesPropertiesWindowFactory()->
      get_properties()->set(*settings.m_time_and_sales_properties);
  }
  auto top_level_widgets = windows;
  for(auto& widget : QApplication::topLevelWidgets()) {
    if(std::find(top_level_widgets.begin(), top_level_widgets.end(), widget) ==
        top_level_widgets.end()) {
      top_level_widgets.push_back(widget);
    }
  }
  for(auto widget : top_level_widgets) {
    if(auto order_imbalance_indicator =
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
  return windows;
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
