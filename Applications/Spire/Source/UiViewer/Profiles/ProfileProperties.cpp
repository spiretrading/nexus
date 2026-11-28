#include "Spire/UiViewer/Profiles/ProfileProperties.hpp"
#include <QFontDatabase>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Styles/Stylist.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

QString Spire::to_string(const Decimal& value) {
  return QString::fromStdString(value.str(
    Decimal::backend_type::cpp_dec_float_digits10, std::ios_base::dec));
}

void Spire::update_widget_style(QWidget& widget,
    const boost::optional<Styles::StyleSheet>& styles) {
  if(!styles) {
    return;
  }
  update_style(widget, [&] (auto& style) {
    for(auto& rule : styles->get_rules()) {
      for(auto i = rule.get_block().begin(); i != rule.get_block().end();
          ++i) {
        style.get(rule.get_selector()).set(*i);
      }
    }
  });
}

void Spire::populate_font_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties,
    const QString& property_name) {
  auto font_database = QFontDatabase();
  auto font1 = font_database.font("Roboto", "Regular", -1);
  font1.setPixelSize(scale_width(12));
  auto font2 = font_database.font("Roboto", "Thin", -1);
  font2.setPixelSize(scale_width(12));
  auto font3 = font1;
  font3.setPixelSize(scale_width(8));
  auto font4 = font_database.font("Tahoma", "Bold", -1);
  font4.setPixelSize(scale_width(16));
  auto font5 = font_database.font("Segoe UI", "Light Italic", -1);
  font5.setPixelSize(scale_width(10));
  auto font_property = define_enum<QFont>(
    {{"Roboto, Regular, 12", font1}, {"Roboto, Thin, 12", font2},
    {"Roboto, Regular, 8", font3}, {"Tahoma, Bold, 16", font4},
    {"Segoe UI, Light Italic, 10", font5}});
  populate_enum_properties(properties, property_name, font_property);
}

boost::optional<boost::posix_time::time_duration>
    Spire::parse_duration(const QString& duration) {
  try {
    return duration_from_string(duration.toStdString().c_str());
  } catch(const std::exception&) {
    return {};
  }
}

const std::vector<std::pair<QString, Qt::Orientation>>&
    Spire::get_orientation_property() {
  static auto property = define_enum<Qt::Orientation>(
    {{"HORIZONTAL", Qt::Orientation::Horizontal},
     {"VERTICAL", Qt::Orientation::Vertical}});
  return property;
}
