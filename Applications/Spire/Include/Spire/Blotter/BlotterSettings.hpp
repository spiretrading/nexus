#ifndef SPIRE_BLOTTER_SETTINGS_HPP
#define SPIRE_BLOTTER_SETTINGS_HPP
#include <Beam/Serialization/ShuttleOptional.hpp>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Spire/Blotter/BlotterWindowSettings.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {
  class BlotterController;

  /** Stores the configuration of a blotter. */
  class BlotterSettings {
    public:

      /** Constructs empty blotter settings. */
      BlotterSettings();

      /**
       * Captures a blotter's configuration.
       * @param controller The blotter to capture.
       */
      explicit BlotterSettings(const BlotterController& controller);

      /**
       * Restores a closed blotter.
       * @param controller The blotter to restore.
       */
      void apply(BlotterController& controller) const;

    private:
      friend struct Beam::DataShuttle;
      friend class BlotterRegistry;
      struct Monitor {
        QString m_name;
        std::unique_ptr<CanvasNode> m_expression;
        boost::optional<int> m_default_width;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };
      boost::uuids::uuid m_id;
      QString m_name;
      Beam::DirectoryEntry m_account;
      bool m_is_consolidated;
      std::vector<boost::uuids::uuid> m_links;
      std::vector<Monitor> m_monitors;
      boost::optional<BlotterWindowSettings> m_window;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BlotterSettings::Monitor::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("expression", m_expression);
    shuttle.shuttle("default_width", m_default_width);
  }

  template<Beam::IsShuttle S>
  void BlotterSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("id", m_id);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("account", m_account);
    shuttle.shuttle("is_consolidated", m_is_consolidated);
    shuttle.shuttle("links", m_links);
    shuttle.shuttle("monitors", m_monitors);
    shuttle.shuttle("window", m_window);
  }
}

#endif
