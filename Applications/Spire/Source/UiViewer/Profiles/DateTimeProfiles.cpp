#include "Spire/UiViewer/Profiles/DateTimeProfiles.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/CalendarDatePicker.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/Ui/DateFilterPanel.hpp"
#include "Spire/Ui/DurationBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto parse_date(const QString& string) -> boost::optional<date> {
    try {
      auto parsed_date = from_string(string.toStdString());
      if(!parsed_date.is_not_a_date()) {
        return parsed_date;
      }
    } catch(const std::exception&) {}
    return {};
  }
}

UiProfile Spire::make_calendar_date_picker_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_date = boost::gregorian::day_clock::local_day();
  properties.push_back(
    make_standard_property("current", to_text(current_date)));
  properties.push_back(make_standard_property(
    "min", to_text(current_date - boost::gregorian::months(2))));
  properties.push_back(make_standard_property(
    "max", to_text(current_date + boost::gregorian::months(2))));
  auto profile = UiProfile("CalendarDatePicker", properties,
    [] (auto& profile) {
      auto model = std::make_shared<LocalOptionalDateModel>();
      auto& current = get<QString>("current", profile.get_properties());
      model->set(parse_date(current.get()));
      auto& min = get<QString>("min", profile.get_properties());
      if(auto min_date = parse_date(min.get())) {
        model->set_minimum(min_date);
      } else {
        model->set_minimum(date(1900, 1, 1));
      }
      auto& max = get<QString>("max", profile.get_properties());
      if(auto max_date = parse_date(max.get())) {
        model->set_maximum(*max_date);
      } else {
        model->set_maximum(date(2100, 12, 31));
      }
      auto calendar = new CalendarDatePicker(model);
      apply_widget_properties(calendar, profile.get_properties());
      current.connect_changed_signal([=] (const auto& value) {
        auto date = parse_date(value);
        if(date && !date->is_not_a_date() && *date != model->get()) {
          model->set(*date);
        }
      });
      calendar->get_current()->connect_update_signal([&current] (auto day) {
        if(day) {
          current.set(to_text(*day));
        }
      });
      calendar->get_current()->connect_update_signal(
        profile.make_event_slot<optional<date>>("Current"));
      calendar->connect_submit_signal(
        profile.make_event_slot<optional<date>>("Submit"));
      return calendar;
    });
  return profile;
}

UiProfile Spire::make_date_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      text_align: left;
      year_field: true;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  auto current_date = day_clock::local_day();
  properties.push_back(
    make_standard_property("current", to_text(current_date)));
  properties.push_back(make_standard_property("read_only", false));
  properties.push_back(
    make_standard_property("min", to_text(current_date - months(2))));
  properties.push_back(
    make_standard_property("max", to_text(current_date + months(2))));
  auto profile = UiProfile("DateBox", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalOptionalDateModel>();
    model->connect_update_signal(
      profile.make_event_slot<optional<date>>("Current"));
    auto& current = get<QString>("current", profile.get_properties());
    model->connect_update_signal([&current] (const auto& value) {
      if(value) {
        current.set(QString::fromStdString(std::to_string(value->year()) +
          "-" + std::to_string(value->month()) + "-" +
          std::to_string(value->day())));
      } else {
        current.set("");
      }
    });
    current.connect_changed_signal([=] (const auto& current) {
      if(current.isEmpty()) {
        if(model->get()) {
          model->set(none);
        }
      } else {
        auto date = parse_date(current);
        if(date && model->get() != date) {
          model->set(date);
        }
      }
    });
    auto& min = get<QString>("min", profile.get_properties());
    min.connect_changed_signal([=] (const auto& min) {
      model->set_minimum(parse_date(min));
    });
    auto& max = get<QString>("max", profile.get_properties());
    max.connect_changed_signal([=] (const auto& max) {
      model->set_maximum(parse_date(max));
    });
    auto date_box = new DateBox(model);
    apply_widget_properties(date_box, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*date_box, styles);
    });
    date_box->connect_submit_signal(
      profile.make_event_slot<optional<date>>("Submit"));
    date_box->connect_reject_signal(
      profile.make_event_slot<optional<date>>("Reject"));
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto value) {
      date_box->set_read_only(value);
    });
    return date_box;
  });
  return profile;
}

UiProfile Spire::make_date_filter_panel_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto current_date = day_clock::local_day();
  properties.push_back(make_standard_property(
    "default_start_date", to_text(current_date - months(3))));
  properties.push_back(
    make_standard_property("default_end_date", to_text(current_date)));
  properties.push_back(make_standard_property("default_offset_value", 1));
  auto unit_property = define_enum<DateFilterPanel::DateUnit>(
    {{"Day", DateFilterPanel::DateUnit::DAY},
     {"Week", DateFilterPanel::DateUnit::WEEK},
     {"Month", DateFilterPanel::DateUnit::MONTH},
     {"Year", DateFilterPanel::DateUnit::YEAR}});
  properties.push_back(make_standard_enum_property(
    "default_date_unit", unit_property));
  auto profile = UiProfile("DateFilterPanel", properties, [] (auto& profile) {
    auto& start_date =
      get<QString>("default_start_date", profile.get_properties());
    auto& end_date = get<QString>("default_end_date", profile.get_properties());
    auto& offset_value =
      get<int>("default_offset_value", profile.get_properties());
    auto& date_unit = get<DateFilterPanel::DateUnit>(
      "default_date_unit", profile.get_properties());
    auto model =
      std::make_shared<LocalValueModel<DateFilterPanel::DateRange>>();
    auto date_range = [&] () -> DateFilterPanel::DateRange {
      auto start = parse_date(start_date.get());
      auto end = parse_date(end_date.get());
      if(start || end) {
        return DateFilterPanel::AbsoluteDateRange{
          start.value_or(date()), end.value_or(date())};
      }
      return DateFilterPanel::RelativeDateRange{
        date_unit.get(), offset_value.get()};
    }();
    model->set(date_range);
    auto panel = new DateFilterPanel(model);
    apply_widget_properties(panel, profile.get_properties());
    panel->get_current()->connect_update_signal(
      profile.make_event_slot<DateFilterPanel::DateRange>("Current",
        [&] (const auto& date_range) {
          auto result = QString();
          std::visit([&] (auto&& date_range) {
            using T = std::decay_t<decltype(date_range)>;
            if constexpr(
                std::is_same_v<T, DateFilterPanel::AbsoluteDateRange>) {
              if(date_range.m_start.is_not_a_date()) {
                result += "none";
              } else {
                result += to_text(date_range.m_start);
              }
              result += " ";
              if(date_range.m_end.is_not_a_date()) {
                result += "none";
              } else {
                result += to_text(date_range.m_end);
              }
            } else if constexpr(
                std::is_same_v<T, DateFilterPanel::RelativeDateRange>) {
              auto to_string = [] (auto unit) {
                if(unit == DateFilterPanel::DateUnit::DAY) {
                  return "Day";
                } else if(unit == DateFilterPanel::DateUnit::WEEK) {
                  return "Week";
                } else if(unit == DateFilterPanel::DateUnit::MONTH) {
                  return "Month";
                } else {
                  return "Year";
                }
              };
              result += QString("%1 %2").
                arg(date_range.m_value).
                arg(to_string(date_range.m_unit));
            }
          }, date_range);
          return result;
        }));
    return panel;
  });
  return profile;
}

UiProfile Spire::make_duration_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      text_align: left;
      format: "hh:mm:ss.fff";
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(
    make_standard_property<QString>("minimum", "10:10:10.000"));
  properties.push_back(
    make_standard_property<QString>("maximum", "20:20:20.000"));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile("DurationBox", properties, [] (auto& profile) {
    auto model = std::make_shared<LocalOptionalDurationModel>();
    auto duration_box = new DurationBox(model);
    apply_widget_properties(duration_box, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*duration_box, styles);
    });
    auto& minimum = get<QString>("minimum", profile.get_properties());
    minimum.connect_changed_signal([=] (auto value) {
      if(auto minimum_value = parse_duration(value)) {
        model->set_minimum(minimum_value);
      }
    });
    auto& maximum = get<QString>("maximum", profile.get_properties());
    maximum.connect_changed_signal([=] (auto value) {
      if(auto maximum_value = parse_duration(value)) {
        model->set_maximum(maximum_value);
      }
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(auto current_value = parse_duration(value)) {
        if(duration_box->get_current()->get() != *current_value) {
          duration_box->get_current()->set(*current_value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      duration_box->set_read_only(is_read_only);
    });
    duration_box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<time_duration>>("Current"));
    duration_box->connect_submit_signal(
      profile.make_event_slot<optional<time_duration>>("Submit"));
    duration_box->connect_reject_signal(
      profile.make_event_slot<optional<time_duration>>("Reject"));
    return duration_box;
  });
  return profile;
}

UiProfile Spire::make_time_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto default_style = R"(
    any {
      text_align: left;
      format: "hh:mm:ss.fff";
    }
  )";
  properties.push_back(make_style_property("style_sheet", default_style));
  properties.push_back(make_standard_property<QString>("current", ""));
  properties.push_back(make_standard_property<bool>("read_only"));
  auto profile = UiProfile("TimeBox", properties, [] (auto& profile) {
    auto parse_time = [] (auto time) -> boost::optional<time_duration> {
      try {
        return boost::posix_time::duration_from_string(
          time.toStdString().c_str());
      } catch(const std::exception&) {
        return {};
      }
    };
    auto time_box = make_time_box();
    apply_widget_properties(time_box, profile.get_properties());
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*time_box, styles);
    });
    auto& current = get<QString>("current", profile.get_properties());
    current.connect_changed_signal([=] (auto value) {
      if(auto current_value = parse_time(value)) {
        if(time_box->get_current()->get() != *current_value) {
          time_box->get_current()->set(*current_value);
        }
      }
    });
    auto& read_only = get<bool>("read_only", profile.get_properties());
    read_only.connect_changed_signal([=] (auto is_read_only) {
      time_box->set_read_only(is_read_only);
    });
    time_box->get_current()->connect_update_signal(
      profile.make_event_slot<optional<time_duration>>("Current"));
    time_box->connect_submit_signal(
      profile.make_event_slot<optional<time_duration>>("Submit"));
    time_box->connect_reject_signal(
      profile.make_event_slot<optional<time_duration>>("Reject"));
    return time_box;
  });
  return profile;
}
