#ifndef SPIRE_TIMERANGEINPUTWIDGETWINDOWSETTINGS_HPP
#define SPIRE_TIMERANGEINPUTWIDGETWINDOWSETTINGS_HPP
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class TimeRangeInputWidgetWindowSettings
      \brief Stores the window settings for a TimeRangeInputWidget.
   */
  class TimeRangeInputWidgetWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a TimeRangeInputWidgetWindowSettings.
      /*!
        \param widget The widget to represent.
      */
      TimeRangeInputWidgetWindowSettings(const TimeRangeInputWidget& widget);

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;

      TimeRangeInputWidgetWindowSettings();
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeRangeInputWidgetWindowSettings::shuttle(S& shuttle,
    unsigned int version) {}
}

#endif
