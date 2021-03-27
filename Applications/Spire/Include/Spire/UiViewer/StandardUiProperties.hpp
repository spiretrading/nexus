#ifndef SPIRE_STANDARD_UI_PROPERTY_HPP
#define SPIRE_STANDARD_UI_PROPERTY_HPP
#include "Nexus/Definitions/Currency.hpp"
#include "Spire/UiViewer/TypedUiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Implements a TypedUiProperty using a user provided factory function to
  //! construct the setter widget.
  /*!
    \tparam T The property's static type.
  */
  template<typename T>
  class StandardUiProperty : public TypedUiProperty<T> {
    public:

      //! The property's static type.
      using Type = typename TypedUiProperty<T>::Type;

      //! Constructs a StandardUiProperty with a default constructed value.
      /*!
        \param name The name of the property, forwarded to the parent
               TypedUiProperty<T>.
        \param setter_factory Constructs the setter widget.
      */
      StandardUiProperty(QString name,
        std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory);

      //! Constructs a StandardUiProperty.
      /*!
        \param name The name of the property, forwarded to the parent
               TypedUiProperty<T>.
        \param value The property's initial value, forwarded to the parent
               TypedUiProperty<T>.
        \param setter_factory Constructs the setter widget.
      */
      StandardUiProperty(QString name, Type value,
        std::function<QWidget* (QWidget*, StandardUiProperty&)> setter_factory);

      QWidget* make_setter_widget(QWidget* parent) override;

    private:
      std::function<QWidget* (QWidget*, StandardUiProperty&)> m_setter_factory;
  };

  //! Populates a list of properties with the basic QWidget properties.
  /*!
    \param properties The list of properties to populate.
  */
  void populate_widget_properties(
    std::vector<std::shared_ptr<UiProperty>>& properties);

  //! Applies the basic QWidget properties to a given QWidget.
  /*!
    \param widget The QWidget having the properties applied to it.
    \param properties The list of properties to apply to the <i>widget</i>.
  */
  void apply_widget_properties(QWidget* widget,
    const std::vector<std::shared_ptr<UiProperty>>& properties);

  //! Returns a standard TypedUiProperty<bool> with an initial value of
  //! <i>false</i>.
  /*!
    \param name The name of the property.
  */
  std::shared_ptr<TypedUiProperty<bool>> make_standard_bool_property(
    QString name);

  //! Returns a standard TypedUiProperty<bool>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<bool>> make_standard_bool_property(
    QString name, bool value);

  //! Returns a standard TypedUiProperty<CurrencyId> with an initial value of
  //! <i>NONE</i>.
  /*!
    \param name The name of the property.
  */
  std::shared_ptr<TypedUiProperty<Nexus::CurrencyId>>
    make_standard_currency_property(QString name);

  //! Returns a standard TypedUiProperty<CurrencyId>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<Nexus::CurrencyId>>
    make_standard_currency_property(QString name, Nexus::CurrencyId value);

  //! Returns a standard TypedUiProperty<int> with an initial value of <i>0</i>.
  /*!
    \param name The name of the property.
  */
  std::shared_ptr<TypedUiProperty<int>> make_standard_int_property(
    QString name);

  //! Returns a standard TypedUiProperty<int>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<int>> make_standard_int_property(QString name,
    int value);

  //! Returns a standard TypedUiProperty<std::int64>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<std::int64_t>> make_standard_int64_property(
    QString name, std::int64_t value);

  //! Returns a standard TypedUiProperty<QColor> with an initial value of white.
  /*!
    \param name The name of the property.
  */
  std::shared_ptr<TypedUiProperty<QColor>> make_standard_qcolor_property(
    QString name);

  //! Returns a standard TypedUiProperty<QColor>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<QColor>> make_standard_qcolor_property(
    QString name, QColor value);

  //! Returns a standard TypedUiProperty<QString> with an empty string as its
  //! initial value.
  /*!
    \param name The name of the property.
  */
  std::shared_ptr<TypedUiProperty<QString>> make_standard_qstring_property(
    QString name);

  //! Returns a standard TypedUiProperty<QString>.
  /*!
    \param name The name of the property.
    \param value The property's initial value.
  */
  std::shared_ptr<TypedUiProperty<QString>> make_standard_qstring_property(
    QString name, QString value);

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
}

#endif
