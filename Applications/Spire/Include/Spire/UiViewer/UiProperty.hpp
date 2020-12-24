#ifndef SPIRE_UI_PROPERTY_HPP
#define SPIRE_UI_PROPERTY_HPP
#include <any>
#include <functional>
#include <QWidget>
#include "Spire/UiViewer/UiViewer.hpp"

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

      //! Disconnects all slots.
      void disconnect();

      //! Resets the property, disconnecting all slots and resetting the value.
      virtual void reset();

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

      //! Signals a change in the property's value.
      void signal_change();

    private:
      mutable ChangedSignal m_changed_signal;
      QString m_name;

      UiProperty(const UiProperty&) = delete;
      UiProperty& operator =(const UiProperty&) = delete;
  };
}

#endif
