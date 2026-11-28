#include "Spire/UiViewer/Profiles/ScrollProfiles.hpp"
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto make_grid_image(const QSize& cell_size, int column_count,
      int row_count) {
    auto image = QImage(QSize(cell_size.width() * column_count,
      cell_size.height() * row_count), QImage::Format_RGB32);
    image.fill(QColor(0x56C4C5));
    auto painter = QPainter(&image);
    for(auto row = 0; row < row_count; ++row) {
      for(auto column = row % 2; column < column_count; column += 2) {
        auto cell_rect = QRect(QPoint(column * cell_size.width(),
          row * cell_size.width()), cell_size);
        painter.fillRect(cell_rect, QColor(0xA2218E));
        painter.fillRect(cell_rect - QMargins(scale_width(1), scale_height(1),
          scale_width(1), scale_height(1)), QColor(0xFDC777));
        painter.setPen(QColor(0x023888));
        cell_rect.translate(
          translate(5, 5) + QPoint(0, painter.fontMetrics().height()));
        painter.drawText(
          cell_rect.topLeft(), QString("(%1, %2)").arg(column).arg(row));
      }
    }
    return image;
  }
}

UiProfile Spire::make_scroll_bar_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  properties.push_back(make_standard_property("vertical", true));
  properties.push_back(make_standard_property("start_range", 0));
  properties.push_back(make_standard_property("end_range", 1000));
  properties.push_back(make_standard_property("page_size", 100));
  properties.push_back(make_standard_property("line_size", 10));
  properties.push_back(make_standard_property("position", 0));
  auto profile = UiProfile("ScrollBar", properties, [] (auto& profile) {
    auto& vertical = get<bool>("vertical", profile.get_properties());
    auto orientation = [&] {
      if(vertical.get()) {
        return Qt::Vertical;
      } else {
        return Qt::Horizontal;
      }
    }();
    auto scroll_bar = new ScrollBar(orientation);
    if(orientation == Qt::Vertical) {
      scroll_bar->setFixedSize(scale(13, 200));
    } else {
      scroll_bar->setFixedSize(scale(200, 13));
    }
    apply_widget_properties(scroll_bar, profile.get_properties());
    auto& start_range = get<int>("start_range", profile.get_properties());
    start_range.connect_changed_signal([scroll_bar] (auto value) {
      auto range = scroll_bar->get_range();
      range.m_start = value;
      scroll_bar->set_range(range);
    });
    auto& end_range = get<int>("end_range", profile.get_properties());
    end_range.connect_changed_signal([scroll_bar] (auto value) {
      auto range = scroll_bar->get_range();
      range.m_end = value;
      scroll_bar->set_range(range);
    });
    auto& page_size = get<int>("page_size", profile.get_properties());
    page_size.connect_changed_signal([scroll_bar] (auto value) {
      scroll_bar->set_page_size(value);
    });
    auto& line_size = get<int>("line_size", profile.get_properties());
    line_size.connect_changed_signal([scroll_bar] (auto value) {
      scroll_bar->set_line_size(value);
    });
    auto& position = get<int>("position", profile.get_properties());
    position.connect_changed_signal([scroll_bar] (auto value) {
      scroll_bar->set_position(value);
    });
    scroll_bar->connect_position_signal(
      profile.make_event_slot<int>("Position"));
    return scroll_bar;
  });
  return profile;
}

UiProfile Spire::make_scroll_box_profile() {
  auto properties = std::vector<std::shared_ptr<UiProperty>>();
  populate_widget_properties(properties);
  auto display_policy_property = define_enum<ScrollBox::DisplayPolicy>(
    {{"NEVER", ScrollBox::DisplayPolicy::NEVER},
     {"ALWAYS", ScrollBox::DisplayPolicy::ALWAYS},
     {"ON_OVERFLOW", ScrollBox::DisplayPolicy::ON_OVERFLOW},
     {"ON_ENGAGE", ScrollBox::DisplayPolicy::ON_ENGAGE}});
  properties.push_back(make_standard_enum_property(
    "horizontal_display_policy", display_policy_property));
  properties.push_back(make_standard_enum_property(
    "vertical_display_policy", display_policy_property));
  auto default_style = R"(
    any {
      horizontal_padding: 10px;
      vertical_padding: 10px;
      border: 1px 0xC8C8C8;
    }
  )";
  properties.push_back(make_style_property("style_sheet",
    std::move(default_style)));
  properties.push_back(make_standard_property("rows", 3));
  properties.push_back(make_standard_property("columns", 3));
  auto profile = UiProfile("ScrollBox", properties, [] (auto& profile) {
    auto label = new QLabel();
    auto& columns = get<int>("columns", profile.get_properties());
    auto& rows = get<int>("rows", profile.get_properties());
    label->setPixmap(QPixmap::fromImage(
      make_grid_image(scale(100, 100), columns.get(), rows.get())));
    auto scroll_box = new ScrollBox(label);
    apply_widget_properties(scroll_box, profile.get_properties());
    auto& horizontal_display_policy = get<ScrollBox::DisplayPolicy>(
      "horizontal_display_policy", profile.get_properties());
    horizontal_display_policy.connect_changed_signal([=] (auto value) {
      scroll_box->set_horizontal(value);
    });
    auto& vertical_display_policy = get<ScrollBox::DisplayPolicy>(
      "vertical_display_policy", profile.get_properties());
    vertical_display_policy.connect_changed_signal([=] (auto value) {
      scroll_box->set_vertical(value);
    });
    auto& style_sheet =
      get<optional<StyleSheet>>("style_sheet", profile.get_properties());
    style_sheet.connect_changed_signal([=] (const auto& styles) {
      update_widget_style(*scroll_box, styles);
    });
    return scroll_box;
  });
  return profile;
}
