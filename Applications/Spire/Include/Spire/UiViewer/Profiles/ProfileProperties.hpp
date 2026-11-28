#ifndef SPIRE_UI_VIEWER_PROFILE_PROPERTIES_HPP
#define SPIRE_UI_VIEWER_PROFILE_PROPERTIES_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Spire/Spire/Decimal.hpp"
#include "Spire/UiViewer/StandardUiProperties.hpp"

namespace Spire {

  /**
   * Keeps a model and a profile property synchronized.
   * @tparam T The value model type.
   * @param model The model whose updates are reflected in the property.
   * @param property The viewer property whose edits update the model.
   */
  template<typename T>
  void link(const std::shared_ptr<T>& model,
      TypedUiProperty<typename T::Type>& property) {
    property.connect_changed_signal([=] (auto value) {
      if(model->get() != value) {
        model->set(value);
      }
    });
    model->connect_update_signal([&] (auto value) {
      if(property.get() != value) {
        property.set(value);
      }
    });
  }

  /**
   * Initializes a property from a widget and applies later edits to the widget.
   * @tparam G The getter member function pointer type.
   * @tparam S The setter member function pointer type.
   * @tparam W The widget type.
   * @tparam T The property value type.
   * @param getter Reads the current value from the widget.
   * @param setter Applies an edited value to the widget.
   * @param widget The widget that owns the setting.
   * @param property The viewer property used to edit the setting.
   */
  template<typename G, typename S, typename W, typename T>
  void link(const G& getter, const S& setter, W& widget,
      TypedUiProperty<T>& property) {
    if((widget.*getter)() != property.get()) {
      property.set((widget.*getter)());
    }
    property.connect_changed_signal([=, &widget] (auto value) {
      if((widget.*getter)() != value) {
        (widget.*setter)(value);
      }
    });
  }

  /**
   * Formats a decimal value for the event log.
   * @param value The decimal to display.
   */
  QString to_string(const Decimal& value);

  /**
   * Applies the rules from a profile style property.
   * @param widget The widget to style.
   * @param styles The rules to apply, if supplied.
   */
  void update_widget_style(
    QWidget& widget, const boost::optional<Styles::StyleSheet>& styles);

  /**
   * Adds an enum property to the profile.
   * @tparam T The type of each selectable value.
   * @param properties The profile properties to append to.
   * @param property_name The name displayed in the property editor.
   * @param property The display label and value for each choice.
   */
  template<typename T>
  void populate_enum_properties(
      std::vector<std::shared_ptr<UiProperty>>& properties,
      const QString& property_name,
      std::vector<std::pair<QString, T>>& property) {
    properties.push_back(make_standard_enum_property(property_name, property));
  }

  /**
   * Adds the sample fonts to a profile.
   * @param properties The profile properties to append to.
   * @param property_name The name displayed for the font selector.
   */
  void populate_font_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties,
    const QString& property_name);

  /**
   * Parses a duration entered in the viewer.
   * @param duration The text entered in the duration field.
   */
  boost::optional<boost::posix_time::time_duration>
    parse_duration(const QString& duration);

  /** Returns the orientation choices for a profile. */
  const std::vector<std::pair<QString, Qt::Orientation>>&
    get_orientation_property();
}

#endif
