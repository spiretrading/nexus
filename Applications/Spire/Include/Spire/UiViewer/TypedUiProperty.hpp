#ifndef SPIRE_TYPED_UI_PROPERTY_HPP
#define SPIRE_TYPED_UI_PROPERTY_HPP
#include <vector>
#include "Spire/UiViewer/UiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {

  //! Provides a type safe derived class of the UiProperty that can be used
  //! as a helper class to implement a property for a specific type.
  /*!
     \tparam T The property's static type.
  */
  template<typename T>
  class TypedUiProperty : public UiProperty {
    public:

      //! The property's static type.
      using Type = T;

      //! Type safe version of the UiProperty's ChangedSignal.
      using ChangedSignal = Signal<void (const Type& value)>;

      //! Returns the property's current value, type-safe version.
      const Type& get() const;

      std::any get_value() const override;

      //! Sets the value of the property.
      void set(const Type& value);

      //! Connects a slot to the type-safe ChangedSignal.
      /*!
        \param slot The slot to connect.
      */
      boost::signals2::connection connect_changed_signal(
        const typename ChangedSignal::slot_type& slot) const;

      void reset() override;

    protected:

      //! Constructs a TypedUiProperty with a default constructed value.
      /*!
        \param name The name of the property, forwarded to the parent
               UiProperty.
      */
      TypedUiProperty(QString name);

      //! Constructs a TypedUiProperty.
      /*!
        \param name The name of the property, forwarded to the parent
               UiProperty.
        \param value The property's initial value.
      */
      TypedUiProperty(QString name, Type value);

    private:
      Type m_initial_value;
      Type m_value;
  };

  //! Finds the type-safe UiProperty with a given name and static type contained
  //! within a collection.
  /*!
    \tparam T The property's static type.
    \param name The name of the property to find.
    \param properties The list of properties to search.
    \return The TypedUiProperty<T> with the given name.
  */
  template<typename T>
  TypedUiProperty<T>& get(const QString& name,
      const std::vector<std::shared_ptr<UiProperty>>& properties) {
    auto property = std::find_if(properties.begin(), properties.end(),
      [&] (const auto& property) {
        return property->get_name() == name &&
          typeid(T) == property->get_value().type();
      });
    return static_cast<TypedUiProperty<T>&>(**property);
  }

  template<typename T>
  const typename TypedUiProperty<T>::Type& TypedUiProperty<T>::get() const {
    return m_value;
  }

  template<typename T>
  std::any TypedUiProperty<T>::get_value() const {
    return get();
  }

  template<typename T>
  void TypedUiProperty<T>::set(const Type& value) {
    m_value = value;
    signal_change();
  }

  template<typename T>
  boost::signals2::connection TypedUiProperty<T>::connect_changed_signal(
      const typename ChangedSignal::slot_type& slot) const {
    return UiProperty::connect_changed_signal(
      [slot] (const std::any& value) {
        slot(std::any_cast<const Type&>(value));
      });
  }

  template<typename T>
  void TypedUiProperty<T>::reset() {
    m_value = m_initial_value;
    UiProperty::reset();
  }

  template<typename T>
  TypedUiProperty<T>::TypedUiProperty(QString name)
    : TypedUiProperty(std::move(name), Type()) {}

  template<typename T>
  TypedUiProperty<T>::TypedUiProperty(QString name, Type value)
    : UiProperty(std::move(name)),
      m_initial_value(std::move(value)),
      m_value(m_initial_value) {}
}

#endif
