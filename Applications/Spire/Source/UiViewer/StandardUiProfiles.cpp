#include "Spire/UiViewer/StandardUiProfiles.hpp"
#include "Spire/UiViewer/Profiles/AccountProfiles.hpp"
#include "Spire/UiViewer/Profiles/BoxProfiles.hpp"
#include "Spire/UiViewer/Profiles/ButtonProfiles.hpp"
#include "Spire/UiViewer/Profiles/ColorProfiles.hpp"
#include "Spire/UiViewer/Profiles/ComboBoxProfiles.hpp"
#include "Spire/UiViewer/Profiles/CurrencyProfiles.hpp"
#include "Spire/UiViewer/Profiles/DateTimeProfiles.hpp"
#include "Spire/UiViewer/Profiles/DestinationProfiles.hpp"
#include "Spire/UiViewer/Profiles/DisplayProfiles.hpp"
#include "Spire/UiViewer/Profiles/FilterPanelProfiles.hpp"
#include "Spire/UiViewer/Profiles/FontProfiles.hpp"
#include "Spire/UiViewer/Profiles/InputBoxProfiles.hpp"
#include "Spire/UiViewer/Profiles/KeyProfiles.hpp"
#include "Spire/UiViewer/Profiles/ListProfiles.hpp"
#include "Spire/UiViewer/Profiles/MenuProfiles.hpp"
#include "Spire/UiViewer/Profiles/NavigationProfiles.hpp"
#include "Spire/UiViewer/Profiles/ObserverProfiles.hpp"
#include "Spire/UiViewer/Profiles/OrderFieldProfiles.hpp"
#include "Spire/UiViewer/Profiles/OverlayProfiles.hpp"
#include "Spire/UiViewer/Profiles/ScalarBoxProfiles.hpp"
#include "Spire/UiViewer/Profiles/ScalarFilterPanelProfiles.hpp"
#include "Spire/UiViewer/Profiles/ScopeProfiles.hpp"
#include "Spire/UiViewer/Profiles/ScrollProfiles.hpp"
#include "Spire/UiViewer/Profiles/SliderProfiles.hpp"
#include "Spire/UiViewer/Profiles/SplitViewProfiles.hpp"
#include "Spire/UiViewer/Profiles/TableProfiles.hpp"
#include "Spire/UiViewer/Profiles/TagProfiles.hpp"
#include "Spire/UiViewer/Profiles/TickerProfiles.hpp"
#include "Spire/UiViewer/Profiles/TradingGroupProfiles.hpp"
#include "Spire/UiViewer/Profiles/VenueProfiles.hpp"
#include "Spire/UiViewer/Profiles/WindowProfiles.hpp"

using namespace Spire;

std::span<const UiProfileFactory> Spire::get_standard_ui_profile_factories() {
  static constexpr UiProfileFactory factories[] = {
    make_account_box_profile,
    make_account_filter_panel_profile,
    make_account_list_box_profile,
    make_account_list_item_profile,
    make_adaptive_box_profile,
    make_box_profile,
    make_calendar_date_picker_profile,
    make_check_box_profile,
    make_check_button_menu_item_profile,
    make_closed_filter_panel_profile,
    make_color_box_profile,
    make_color_code_panel_profile,
    make_color_picker_profile,
    make_combo_box_profile,
    make_context_menu_profile,
    make_currency_box_profile,
    make_currency_filter_panel_profile,
    make_currency_list_box_profile,
    make_currency_list_item_profile,
    make_date_box_profile,
    make_date_filter_panel_profile,
    make_decimal_box_profile,
    make_decimal_filter_panel_profile,
    make_deletable_list_item_profile,
    make_delete_icon_button_profile,
    make_destination_box_profile,
    make_destination_filter_panel_profile,
    make_destination_list_box_profile,
    make_destination_list_item_profile,
    make_drop_down_box_profile,
    make_drop_down_list_profile,
    make_duration_box_profile,
    make_duration_filter_panel_profile,
    make_eye_dropper_profile,
    make_editable_box_profile,
    make_editable_table_view_profile,
    make_filter_panel_profile,
    make_focus_observer_profile,
    make_font_box_profile,
    make_font_family_box_profile,
    make_font_style_box_profile,
    make_grouped_table_view_profile,
    make_hex_color_box_profile,
    make_highlight_box_profile,
    make_highlight_picker_profile,
    make_highlight_swatch_profile,
    make_hover_observer_profile,
    make_icon_button_profile,
    make_icon_toggle_button_profile,
    make_identicon_profile,
    make_info_panel_profile,
    make_info_tip_profile,
    make_input_box_profile,
    make_integer_box_profile,
    make_integer_filter_panel_profile,
    make_key_filter_panel_profile,
    make_key_input_box_profile,
    make_key_list_box_profile,
    make_key_tag_profile,
    make_label_button_profile,
    make_label_profile,
    make_list_item_profile,
    make_list_view_profile,
    make_menu_button_profile,
    make_menu_icon_button_profile,
    make_menu_label_button_profile,
    make_money_box_profile,
    make_money_filter_panel_profile,
    make_navigation_view_profile,
    make_number_label_profile,
    make_open_filter_panel_profile,
    make_order_field_info_tip_profile,
    make_order_status_box_profile,
    make_order_status_filter_panel_profile,
    make_order_status_list_box_profile,
    make_order_type_box_profile,
    make_order_type_filter_panel_profile,
    make_overlay_panel_profile,
    make_percent_box_profile,
    make_percent_filter_panel_profile,
    make_popup_box_profile,
    make_position_side_box_profile,
    make_position_side_filter_panel_profile,
    make_progress_bar_profile,
    make_quantity_box_profile,
    make_quantity_filter_panel_profile,
    make_radio_button_profile,
    make_responsive_label_profile,
    make_scope_box_profile,
    make_scope_drop_down_box_profile,
    make_scope_filter_panel_profile,
    make_scope_list_item_profile,
    make_scroll_bar_profile,
    make_scroll_box_profile,
    make_scrollable_list_box_profile,
    make_search_box_profile,
    make_side_box_profile,
    make_side_filter_panel_profile,
    make_slider_profile,
    make_slider_2d_profile,
    make_split_view_profile,
    make_switch_button_profile,
    make_tab_view_profile,
    make_table_header_profile,
    make_table_header_item_profile,
    make_table_view_profile,
    make_tag_profile,
    make_tag_box_profile,
    make_tag_combo_box_profile,
    make_task_state_box_profile,
    make_text_area_box_profile,
    make_text_area_label_profile,
    make_text_box_profile,
    make_ticker_box_profile,
    make_ticker_filter_panel_profile,
    make_ticker_list_box_profile,
    make_ticker_list_item_profile,
    make_ticker_view_profile,
    make_time_box_profile,
    make_time_in_force_box_profile,
    make_time_in_force_filter_panel_profile,
    make_title_bar_profile,
    make_tooltip_profile,
    make_trading_group_box_profile,
    make_trading_group_filter_panel_profile,
    make_trading_group_list_box_profile,
    make_transition_view_profile,
    make_venue_box_profile,
    make_venue_filter_panel_profile,
    make_venue_list_box_profile,
    make_window_highlight_profile
  };
  return factories;
}
