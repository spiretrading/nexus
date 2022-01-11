#ifndef SPIRE_UI_PROFILE_HPP
#define SPIRE_UI_PROFILE_HPP
#include <any>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QWidget>
#include "Spire/UiViewer/UiProperty.hpp"
#include "Spire/UiViewer/UiViewer.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct is_optional : std::false_type {};

  template<typename T>
  struct is_optional<boost::optional<T>> : std::true_type {};

  template<typename T>
  std::any convert(const T& value) {
    return value;
  }

  template<typename T, typename Converter, typename... Converters>
  std::any convert(const T& value, const Converter& converter,
      const Converters&... converters) {
    if constexpr(std::is_invocable_v<Converter, const T&>) {
      return converter(value);
    }
    return convert(value, converters...);
  }
}

  /**
   * Stores the information needed to display a single widget in the UI Viewer.
   */
  class UiProfile {
    public:

      /**
       * Provides a generic signal to indicate a change in the widget.
       * @param name The name of the event.
       * @param arguments The arguments passed to the event.
       */
      using EventSignal = Signal<
        void (const QString& name, const std::vector<std::any>& arguments)>;

      /**
       * Constructs the profile for a widget.
       * @param name The name of the widget.
       * @param properties The widget's properties.
       * @param factory The function used to construct the widget.
       */
      UiProfile(
        QString name, std::vector<std::shared_ptr<UiProperty>> properties,
        std::function<QWidget* (UiProfile&)>);

      UiProfile(UiProfile&&) = default;

      /** Returns the name of the widget. */
      const QString& get_name() const;

      /** Returns the widget's properties. */
      const std::vector<std::shared_ptr<UiProperty>>& get_properties() const;

      /** Returns the widget to display. */
      QWidget* get_widget();

      /**
       * Returns a slot that is used to report updates to the displayed widget.
       */
      template<typename... Args, typename... Converters>
      std::function<void (const Args&...)> make_event_slot(
        const QString& name, const Converters&... converters);

      /**
       * Removes the widget and disconnects the properties' signals without
       * removing the current properties.
       */
      void remove_widget();

      /** Resets this profile. */
      void reset();

      /** Connects a slot to the EventSignal. */
      boost::signals2::connection connect_event_signal(
        const EventSignal::slot_type& slot) const;

      UiProfile& operator =(UiProfile&&) = default;

    private:
      mutable std::shared_ptr<EventSignal> m_event_signal;
      QString m_name;
      std::vector<std::shared_ptr<UiProperty>> m_properties;
      std::function<QWidget* (UiProfile&)> m_factory;
      QWidget* m_widget;

      UiProfile(const UiProfile&) = delete;
      UiProfile& operator =(const UiProfile&) = delete;
  };

  template<typename... Args, typename... Converters>
  std::function<void (const Args&...)> UiProfile::make_event_slot(
      const QString& name, const Converters&... converters) {
    return [=, event_signal = m_event_signal] (const auto&... args) {
      auto values = std::vector<std::any>();
      (values.push_back(
        [&] () -> std::any {
          if constexpr(
              Details::is_optional<std::decay_t<decltype(args)>>::value) {
            if(args) {
              return Details::convert(*args, converters...);
            }
            return nullptr;
          } else {
            return Details::convert(args, converters...);
          }
        }()), ...);
      (*event_signal)(name, values);
    };
  }
}

#endif
