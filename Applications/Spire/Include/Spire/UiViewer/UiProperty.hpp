#ifndef SPIRE_UI_PROPERTY_HPP
#define SPIRE_UI_PROPERTY_HPP
#include <any>
#include <functional>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Base class used to represent a widget's editable property. Handles updates
  //! to the property as well as a factory function to instantiate a widget to
  //! modify the property.
  class UiProperty {
    public:

      //! Signals a change to the property.
      /*!
        \param value The current value.
      */
      using ChangedSignal = Signal<void (const std::any& value)>;

      virtual ~UiProperty() = default;

      //! Returns the name of the property.
      const QString& get_name() const;

      //! Returns the property's current value.
      virtual std::any get_value() const = 0;

      //! Returns a widget that can be used to edit the property.
      virtual QWidget* make_setter_widget(QWidget* parent) = 0;

      //! Attaches this property to a widget's setter function.
      /*!
        \tparam T The static type of the property being set.
        \param setter The setter function to attach this property to.
      */
      template<typename T, typename F>
      void attach(F&& setter);

      //! Connects a slot to the ChangedSignal.
      /*!
        \param slot The slot to connect.
      */
      boost::signals2::connection connect_changed_signal(
        const ChangedSignal::slot_type& slot) const;

    protected:

      //! Constructs a UiProperty.
      /*!
        \param name The name of the property.
      */
      explicit UiProperty(QString name);

      //! Attaches this property to a widget's setter function.
      /*!
        \param setter The type erased std::function<void (const T&)> setter
               to attach this property to.
      */
      virtual void attach_setter(const std::any& setter) = 0;

    private:
      mutable ChangedSignal m_changed_signal;
      QString m_name;

      UiProperty(const UiProperty&) = delete;
      UiProperty& operator =(const UiProperty&) = delete;
  };

  template<typename T, typename F>
  void UiProperty::attach(F&& setter) {
    attach_setter(std::function<void (const T&)>(std::forward<F>(setter)));
  }
}

#endif
