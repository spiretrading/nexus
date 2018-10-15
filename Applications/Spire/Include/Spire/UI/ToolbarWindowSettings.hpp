#ifndef SPIRE_TOOLBARWINDOWSETTINGS_HPP
#define SPIRE_TOOLBARWINDOWSETTINGS_HPP
#include <Beam/Serialization/Serialization.hpp>
#include <QPoint>
#include "Spire/Spire/Spire.hpp"
#include "Spire/UI/WindowSettings.hpp"
#include "Spire/UI/ShuttleQtTypes.hpp"

namespace Spire {
namespace UI {

  /*! \class ToolbarWindowSettings
      \brief Stores the window settings for a Toolbar.
   */
  class ToolbarWindowSettings : public WindowSettings {
    public:

      //! Constructs a ToolbarWindowSettings with default values.
      ToolbarWindowSettings();

      //! Constructs a ToolbarWindowSettings.
      /*!
        \param toolbar The Toolbar to represent.
      */
      ToolbarWindowSettings(const Toolbar& toolbar);

      virtual ~ToolbarWindowSettings();

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> profile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      QPoint m_position;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void ToolbarWindowSettings::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("position", m_position);
  }
}
}

#endif
