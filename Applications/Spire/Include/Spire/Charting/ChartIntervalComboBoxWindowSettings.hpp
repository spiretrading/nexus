#ifndef SPIRE_CHARTINTERVALCOMBOBOXWINDOWSETTINGS_HPP
#define SPIRE_CHARTINTERVALCOMBOBOXWINDOWSETTINGS_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleSharedPtr.hpp>
#include <QByteArray>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class ChartIntervalComboBoxWindowSettings
      \brief Stores the window settings for a ChartIntervalComboBox.
   */
  class ChartIntervalComboBoxWindowSettings : public LegacyUI::WindowSettings {
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
      friend struct Beam::Serialization::DataShuttle;
      std::shared_ptr<NativeType> m_type;
      ChartValue m_value;
      QByteArray m_geometry;

      ChartIntervalComboBoxWindowSettings() = default;
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ChartIntervalComboBoxWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("type", m_type);
    shuttle.Shuttle("value", m_value);
    shuttle.Shuttle("geometry", m_geometry);
  }
}

#endif
