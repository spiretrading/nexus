#ifndef SPIRE_TYPED_UI_PROPERTY_HPP
#define SPIRE_TYPED_UI_PROPERTY_HPP
#include "Spire/UiViewer/UiProperty.hpp"

namespace Spire {

  //! Provides a type safe derived class of the UiProperty that can be used
  //! as a helper class to implement a property for a specific type.
  //! \tparam T The property's static type.
  template<typename T>
  class TypedUiProperty : public UiProperty {
    public:

      //! The property's static type.
      using Type = T;

    protected:

      //! Constructs a TypedUiProperty with a no-op setter.
      /*!
        \param name The name of the property, forwarded to the parent
               UiProperty.
      */
      TypedUiProperty(QString name);

      //! Sets the value of the widget.
      void set(const Type& value);

      void attach_setter(const std::any& setter) override;

    private:
      std::function<void (const Type&)> m_setter;
  };

  template<typename T>
  TypedUiProperty<T>::TypedUiProperty(QString name)
    : UiProperty(std::move(name)),
      m_setter([] (const Type&) {}) {}

  template<typename T>
  void TypedUiProperty<T>::set(const Type& value) {
    m_setter(value);
  }

  template<typename T>
  void TypedUiProperty<T>::attach_setter(const std::any& setter) {
    m_setter = std::any_cast<const std::function<void (const Type&)>&>(setter);
  }
}

#endif
