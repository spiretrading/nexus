#ifndef SPIRE_TIMERANGEINPUTWIDGETWINDOWSETTINGS_HPP
#define SPIRE_TIMERANGEINPUTWIDGETWINDOWSETTINGS_HPP
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"

namespace Spire {

  /*! \class TimeRangeInputWidgetWindowSettings
      \brief Stores the window settings for a TimeRangeInputWidget.
   */
  class TimeRangeInputWidgetWindowSettings : public LegacyUI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;

      TimeRangeInputWidgetWindowSettings();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeRangeInputWidgetWindowSettings::Shuttle(Shuttler& shuttle,
    unsigned int version) {}
}

#endif
