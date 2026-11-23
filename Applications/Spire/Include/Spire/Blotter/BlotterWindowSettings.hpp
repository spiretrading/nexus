#ifndef SPIRE_BLOTTER_WINDOW_SETTINGS_HPP
#define SPIRE_BLOTTER_WINDOW_SETTINGS_HPP
#include <vector>
#include <Beam/Serialization/ShuttleUuid.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/LegacyUI/WindowSettings.hpp"
#include "Spire/Spire/ShuttleQtTypes.hpp"

namespace Spire {
  class BlotterWindow;
  class TableView;

  /** Stores a blotter window's layout. */
  class BlotterWindowSettings : public LegacyUI::WindowSettings {
    public:

      /** Constructs default blotter window settings. */
      BlotterWindowSettings();

      /**
       * Captures a blotter window's layout.
       * @param window The window to capture.
       */
      explicit BlotterWindowSettings(const BlotterWindow& window);

      std::string GetName() const override;
      QWidget* Reopen(Beam::Ref<UserProfile> profile) const override;
      void Apply(Beam::Ref<UserProfile> profile,
        Beam::Out<QWidget> widget) const override;

    private:
      friend struct Beam::DataShuttle;
      struct Column {
        int m_index = 0;
        int m_width = 0;
        bool m_is_visible = true;
        int m_order = 0;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };
      struct Sort {
        int m_index = 0;
        int m_order = 0;

        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };
      struct TableState {
        std::vector<Column> m_columns;
        std::vector<Sort> m_sort;

        TableState() = default;
        explicit TableState(TableView& table);

        void apply(TableView& table) const;
        template<Beam::IsShuttle S>
        void shuttle(S& shuttle, unsigned int version);
      };
      boost::uuids::uuid m_id;
      std::string m_name;
      QByteArray m_geometry;
      double m_split;
      int m_tab;
      TableState m_tasks;
      TableState m_positions;
      TableState m_orders;
      TableState m_executions;
      std::vector<Nexus::OrderStatus> m_statuses;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void BlotterWindowSettings::Column::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("index", m_index);
    shuttle.shuttle("width", m_width);
    shuttle.shuttle("is_visible", m_is_visible);
    shuttle.shuttle("order", m_order);
  }

  template<Beam::IsShuttle S>
  void BlotterWindowSettings::Sort::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("index", m_index);
    shuttle.shuttle("order", m_order);
  }

  template<Beam::IsShuttle S>
  void BlotterWindowSettings::TableState::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("columns", m_columns);
    shuttle.shuttle("sort", m_sort);
  }

  template<Beam::IsShuttle S>
  void BlotterWindowSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("id", m_id);
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("geometry", m_geometry);
    shuttle.shuttle("split", m_split);
    shuttle.shuttle("tab", m_tab);
    shuttle.shuttle("tasks", m_tasks);
    shuttle.shuttle("positions", m_positions);
    shuttle.shuttle("orders", m_orders);
    shuttle.shuttle("executions", m_executions);
    shuttle.shuttle("statuses", m_statuses);
  }
}

#endif
