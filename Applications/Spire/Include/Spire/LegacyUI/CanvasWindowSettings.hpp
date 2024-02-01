#ifndef SPIRE_CANVASWINDOWSETTINGS_HPP
#define SPIRE_CANVASWINDOWSETTINGS_HPP
#include <tuple>
#include <Beam/Serialization/ShuttleTuple.hpp>
#include <QByteArray>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/CanvasView/CanvasNodeModel.hpp"
#include "Spire/LegacyUI/ShuttleQtTypes.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {
namespace LegacyUI {

  /*! \class CanvasWindowSettings
      \brief Stores the window settings for a CanvasWindow.
   */
  class CanvasWindowSettings : public WindowSettings {
    public:

      //! Constructs a CanvasWindowSettings with default values.
      CanvasWindowSettings();

      //! Constructs a CanvasWindowSettings.
      /*!
        \param window The CanvasWindow to represent.
      */
      CanvasWindowSettings(const CanvasWindow& window);

      virtual std::string GetName() const;

      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const;

      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      std::vector<std::tuple<CanvasNodeModel::Coordinate,
        std::unique_ptr<CanvasNode>>> m_nodes;
      QByteArray m_geometry;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void CanvasWindowSettings::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    shuttle.Shuttle("nodes", m_nodes);
    shuttle.Shuttle("geometry", m_geometry);
  }
}
}

#endif
