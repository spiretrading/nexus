#ifndef SPIRE_COLLAPSIBLEWIDGETWINDOWSETTINGS_HPP
#define SPIRE_COLLAPSIBLEWIDGETWINDOWSETTINGS_HPP
#include <QByteArray>
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"
#include "Spire/UI/UI.hpp"
#include "Spire/UI/WindowSettings.hpp"

namespace Spire {
namespace UI {

  /*! \class CollapsibleWidgetWindowSettings
      \brief Stores the window settings for a CollapsibleWidget.
   */
  class CollapsibleWidgetWindowSettings : public UI::WindowSettings {
    public:

      //! Constructs a CollapsibleWidgetWindowSettings with default values.
      CollapsibleWidgetWindowSettings();

      //! Constructs a CollapsibleWidgetWindowSettings.
      /*!
        \param widget The CollapsibleWidget to represent.
      */
      CollapsibleWidgetWindowSettings(const CollapsibleWidget& widget);

      virtual ~CollapsibleWidgetWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::string m_label;
      bool m_isExpanded;
      QByteArray m_geometry;
      std::unique_ptr<WindowSettings> m_subWindowSettings;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CollapsibleWidgetWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("label", m_label);
    shuttle.Shuttle("is_expanded", m_isExpanded);
    shuttle.Shuttle("geometry", m_geometry);
    shuttle.Shuttle("sub_window_settings", m_subWindowSettings);
  }
}
}

#endif
