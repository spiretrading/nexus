#ifndef SPIRE_STANDARD_UI_PROPERTY_HPP
#define SPIRE_STANDARD_UI_PROPERTY_HPP
#include <utility>
#include <vector>
#include <QComboBox>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Styles/StyleSheet.hpp"
#include "Spire/Ui/DateBox.hpp"
#include "Spire/UiViewer/TypedUiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  /**
   * Implements a TypedUiProperty using a user provided factory function to
   * construct the setter widget.
   * @param <T> The property's static type.
   */
  template<typename T>
  class StandardUiProperty : public TypedUiProperty<T> {
    public:

      /** The property's static type. */
      using Type = typename TypedUiProperty<T>::Type;

      /**
       * Constructs a StandardUiProperty with a default constructed value.
       * @param name The name of the property, forwarded to the parent
       *        TypedUiProperty<T>.
       * @param setter_factory Constructs the setter widget.
       */
      StandardUiProperty(QString name,
        std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory);

      /**
       * Constructs a StandardUiProperty.
       * @param name The name of the property, forwarded to the parent
       *        TypedUiProperty<T>.
       * @param value The property's initial value, forwarded to the parent
       *        TypedUiProperty<T>.
       * @param setter_factory Constructs the setter widget.
       */
      StandardUiProperty(QString name, Type value,
        std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory);

      QWidget* make_setter_widget(QWidget* parent) override;

    private:
      std::function<QWidget* (QWidget*, StandardUiProperty&)> m_setter_factory;
  };

  /**
   * Populates width and height properties.
   * @param width_name The name of the width property.
   * @param height_name The name of the height property.
   * @param properties The size properties to populate.
   */
  void populate_widget_size_properties(const QString& width_name,
    const QString& height_name,
    std::vector<std::shared_ptr<UiProperty>>& properties);

  /**
   * Applies the width and height properties to a given QWidget.
   * @param widget The QWidget having the properties applied to it.
   * @param width_name The name of the width property.
   * @param height_name The name of the height property.
   * @param properties The size properties to apply to the <i>widget</i>.
   */
  void apply_widget_size_properties(QWidget* widget,
    const QString& width_name, const QString& height_name,
    const std::vector<std::shared_ptr<UiProperty>>& properties);

  /**
   * Populates a list of properties with the basic QWidget properties.
   * @param properties The list of properties to populate.
   */
  void populate_widget_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties);

  /**
   * Applies the basic QWidget properties to a given QWidget.
   * @param widget The QWidget having the properties applied to it.
   * @param properties The list of properties to apply to the <i>widget</i>.
   */
  void apply_widget_properties(QWidget* widget,
    const std::vector<std::shared_ptr<UiProperty>>& properties);

  /**
   * Returns a standard TypedUiProperty<T>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<typename T>
  std::shared_ptr<TypedUiProperty<T>>
    make_standard_property(QString name, T value);

  /**
   * Returns a standard TypedUiProperty<T> with an default initial value.
   * @param name The name of the property.
   */
  template<typename T>
  std::shared_ptr<TypedUiProperty<T>> make_standard_property(QString name) {
    return make_standard_property(std::move(name), T());
  }

  /**
   * Returns a standard TypedUiProperty<bool>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<bool>>
    make_standard_property<bool>(QString name, bool value);

  /**
   * Returns a standard TypedUiProperty<CurrencyId>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<Nexus::CurrencyId>>
    make_standard_property<Nexus::CurrencyId>(
      QString name, Nexus::CurrencyId value);

  /**
   * Helper function to perform type deduction on an initializer list for
   * a list of pairs associating with each enum constant, its display name.
   * @param definition The initializer list of the form
   *        <code>{{NAME, VALUE}...}</code>.
   */
  template<typename E>
  std::vector<std::pair<QString, E>> define_enum(
      std::vector<std::pair<QString, E>> definition) {
    return definition;
  }

  /**
   * Returns a standard TypedUiProperty for an enumerator type.
   * @param name The name of the property.
   * @param value The property's initial value.
   * @param definition A list of pairs associating with each enum constant, its
   *        display name.
   */
  template<typename E>
  std::shared_ptr<TypedUiProperty<E>> make_standard_enum_property(
      QString name, E value, std::vector<std::pair<QString, E>> definition) {
    return std::make_shared<StandardUiProperty<E>>(std::move(name), value,
      [definition = std::move(definition)] (
          QWidget* parent, StandardUiProperty<E>& property) {
        auto setter = new QComboBox(parent);
        for(auto& entry : definition) {
          setter->addItem(entry.first);
        }
        property.connect_changed_signal([=] (auto value) {
          auto i = std::find_if(definition.begin(), definition.end(),
            [&] (const auto& e) { return value == e.second; });
          auto index = std::distance(definition.begin(), i);
          setter->setCurrentIndex(static_cast<int>(index));
        });
        QObject::connect(setter,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          [&, definition] (auto index) {
            property.set(definition[index].second);
          });
        return setter;
      });
  }

  /**
   * Returns a standard TypedUiProperty for an enumerator type.
   * @param name The name of the property.
   * @param definition A list of pairs associating with each enum constant, its
   *        display name.
   */
  template<typename E>
  std::shared_ptr<TypedUiProperty<E>> make_standard_enum_property(
      QString name, std::vector<std::pair<QString, E>> definition) {
    auto value = definition.front().second;
    return make_standard_enum_property<E>(
      std::move(name), value, std::move(definition));
  }

  /**
   * Returns a standard TypedUiProperty<int>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<int>>
    make_standard_property<int>(QString name, int value);

  /**
   * Returns a standard TypedUiProperty<std::int64>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<std::int64_t>>
    make_standard_property<std::int64_t>(QString name, std::int64_t value);

  /**
   * Returns a standard TypedUiProperty<Nexus::Money>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<Nexus::Money>>
    make_standard_property<Nexus::Money>(QString name, Nexus::Money value);

  /**
   * Returns a standard TypedUiProperty<Nexus::Quantity>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<Nexus::Quantity>>
    make_standard_property<Nexus::Quantity>(QString name,
      Nexus::Quantity value);

  template<>
  std::shared_ptr<TypedUiProperty<Decimal>>
    make_standard_property<Decimal>(QString name, Decimal value);

  /**
   * Returns a standard TypedUiProperty<QColor> with an initial value of white.
   * @param name The name of the property.
   */
  template<>
  std::shared_ptr<TypedUiProperty<QColor>>
    make_standard_property<QColor>(QString name);

  /**
   * Returns a standard TypedUiProperty<QColor>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<QColor>>
    make_standard_property<QColor>(QString name, QColor value);

  /**
   * Returns a standard TypedUiProperty<QString>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<QString>>
    make_standard_property<QString>(QString name, QString value);

  /**
   * Returns a standard TypedUiProperty<Styles::DateFormat>.
   * @param name The name of the property.
   * @param value The property's initial value.
   */
  template<>
  std::shared_ptr<TypedUiProperty<Styles::DateFormat>>
    make_standard_property<Styles::DateFormat>(
      QString name, Styles::DateFormat value);

  template<typename T>
  StandardUiProperty<T>::StandardUiProperty(QString name,
    std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory)
    : TypedUiProperty<T>(std::move(name)),
      m_setter_factory(std::move(setter_factory)) {}

  template<typename T>
  StandardUiProperty<T>::StandardUiProperty(QString name, Type value,
    std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory)
    : TypedUiProperty<T>(std::move(name), std::move(value)),
      m_setter_factory(std::move(setter_factory)) {}

  template<typename T>
  QWidget* StandardUiProperty<T>::make_setter_widget(QWidget* parent) {
    return m_setter_factory(parent, *this);
  }

  std::shared_ptr<TypedUiProperty<Spire::Styles::StyleSheet>>
    make_style_property(QString name, QString style_text);
}

#endif
