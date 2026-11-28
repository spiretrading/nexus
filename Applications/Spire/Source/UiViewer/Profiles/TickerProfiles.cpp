#include "Spire/UiViewer/Profiles/TickerProfiles.hpp"
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/TickerBox.hpp"
#include "Spire/Ui/TickerFilterPanel.hpp"
#include "Spire/Ui/TickerListBox.hpp"
#include "Spire/Ui/TickerListItem.hpp"
#include "Spire/Ui/TickerView.hpp"
#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Spire/UiViewer/Profiles/QueryProfileBuilders.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

UiProfile Spire::make_ticker_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property<QString>("current", "MX.TSX"));
  properties.push_back(make_standard_property<QString>("placeholder"));
  properties.push_back(make_standard_property("read_only", false));
  auto profile = UiProfile("TickerBox", properties, [] (auto& profile) {
    auto model = populate_ticker_query_model();
    auto& current = get<QString>("current", profile.get_properties());
    auto ticker = [&] {
      if(auto value = model->parse(current.get())) {
        return value->m_ticker;
      }
      return Ticker();
    }();
    auto current_model = make_local_value_model(ticker);
    auto box = new TickerBox(model, current_model);
    box->setFixedWidth(scale_width(112));
    apply_widget_properties(box, profile.get_properties());
    auto current_connection = box->get_current()->connect_update_signal(
      profile.make_event_slot<Ticker>("Current"));
    current.connect_changed_signal([=] (const auto& current) {
      if(auto value = model->parse(current)) {
        box->get_current()->set(value->m_ticker);
      } else {
        auto current_blocker = shared_connection_block(current_connection);
        box->get_current()->set(Ticker());
      }
    });
    auto& placeholder = get<QString>("placeholder", profile.get_properties());
    placeholder.connect_changed_signal([=] (const auto& placeholder) {
      box->set_placeholder(placeholder);
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal(
      std::bind_front(&TickerBox::set_read_only, box));
    box->connect_submit_signal(profile.make_event_slot<Ticker>("Submit"));
    return box;
  });
  return profile;
}

UiProfile Spire::make_ticker_filter_panel_profile() {
  return setup_open_filter_panel_profile("TickerFilterPanel",
    [] { return make_ticker_filter_panel(populate_ticker_query_model()); });
}

UiProfile Spire::make_ticker_list_box_profile() {
  return setup_tag_combo_box_profile("TickerListBox",
    [] { return make_ticker_list_box(populate_ticker_query_model()); });
}

UiProfile Spire::make_ticker_list_item_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto profile = UiProfile("TickerListItem", properties, [] (auto& profile) {
    auto ticker = parse_ticker("MRU.TSX");
    auto ticker_info = TickerInfo(ticker, "Metro Inc.", "", 0);
    auto item = new TickerListItem(ticker_info);
    apply_widget_properties(item, profile.get_properties());
    return item;
  });
  return profile;
}

UiProfile Spire::make_ticker_view_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  properties.push_back(make_standard_property<int>("width", 266));
  properties.push_back(make_standard_property<int>("height", 361));
  auto profile = UiProfile("TickerView", properties, [] (auto& profile) {
    auto model = populate_ticker_query_model();
    auto label = make_label("");
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto ticker_view = new TickerView(model, *label);
    auto box = new Box(ticker_view);
    update_style(*box, [] (auto& style) {
      style.get(Hover() || Focus()).
        set(border(scale_width(1), QColor(0x4B23A0)));
    });
    ticker_view->get_current()->connect_update_signal(
      profile.make_event_slot<Ticker>("Current", [=] (const auto& ticker) {
        label->get_current()->set(to_text(ticker));
        return ticker;
      }));
    auto& width = get<int>("width", profile.get_properties());
    width.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_width(ticker_view->width()) != value) {
          ticker_view->setFixedWidth(scale_width(value));
        }
      }
    });
    auto& height = get<int>("height", profile.get_properties());
    height.connect_changed_signal([=] (auto value) {
      if(value != 0) {
        if(unscale_height(ticker_view->height()) != value) {
          ticker_view->setFixedHeight(scale_height(value));
        }
      }
    });
    return box;
  });
  return profile;
}
