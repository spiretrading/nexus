#ifndef SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_SETTINGS_HPP
#include <vector>
#include <QByteArray>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/LegacyUI/SecurityViewStack.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/SecurityView.hpp"

namespace Spire {
  class TimeAndSalesWindow;

  /** Stores the window settings for a TimeAndSalesWindow. */
  class TimeAndSalesWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Constructs a TimeAndSalesWindowSettings with default values. */
      TimeAndSalesWindowSettings() = default;

      /**
       * Constructs a TimeAndSalesWindowSettings.
       * @param window The TimeAndSalesWindow to represent.
       */
      explicit TimeAndSalesWindowSettings(const TimeAndSalesWindow& window);

      std::string GetName() const override;
      QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const override;
      void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      std::string m_name;
      SecurityView::State m_security_view;
      std::vector<int> m_column_widths;
      std::string m_identifier;
      std::string m_link_identifier;
      QByteArray m_geometry;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void TimeAndSalesWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("security_view", m_security_view);
    shuttle.shuttle("column_widths", m_column_widths);
    shuttle.shuttle("identifier", m_identifier);
    shuttle.shuttle("link_identifier", m_link_identifier);
    shuttle.shuttle("geometry", m_geometry);
  }
}

#endif
