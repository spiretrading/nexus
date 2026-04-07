#ifndef SPIRE_CHARTINTERVALCOMBOBOXWINDOWSETTINGS_HPP
#define SPIRE_CHARTINTERVALCOMBOBOXWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <QByteArray>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {

  /*! \class ChartIntervalComboBoxWindowSettings
      \brief Stores the window settings for a ChartIntervalComboBox.
   */
  class ChartIntervalComboBoxWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a ChartIntervalComboBoxWindowSettings.
      /*!
        \param widget The widget to represent.
      */
      ChartIntervalComboBoxWindowSettings(const ChartIntervalComboBox& widget);

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::DataShuttle;
      std::shared_ptr<NativeType> m_type;
      ChartValue m_value;
      QByteArray m_geometry;

      ChartIntervalComboBoxWindowSettings() = default;
      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void ChartIntervalComboBoxWindowSettings::shuttle(S& shuttle,
      unsigned int version) {
    shuttle.shuttle("type", m_type);
    shuttle.shuttle("value", m_value);
    shuttle.shuttle("geometry", m_geometry);
  }
}

#endif
