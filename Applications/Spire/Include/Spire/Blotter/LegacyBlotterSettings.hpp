#ifndef SPIRE_LEGACY_BLOTTER_SETTINGS_HPP
#define SPIRE_LEGACY_BLOTTER_SETTINGS_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Collections/EnumSet.hpp>
#include <Beam/Serialization/ShuttleBitset.hpp>
#include <Beam/Serialization/ShuttleUniquePtr.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"

namespace Spire {

  /**
   * Stores the legacy blotter settings. Provided for backward compatibility
   * with Spire Classic.
   */
  struct LegacyBlotterSettings {

    /** Stores a task monitor used by Spire Classic. */
    struct TaskMonitor {

      /** The name of the monitor. */
      std::string m_name;

      /** The monitor attached to a task. */
      std::unique_ptr<CanvasNode> m_monitor;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
    };

    /** Stores the task properties used by Spire Classic. */
    struct TaskProperties {

      /** The task monitors to display. */
      std::vector<TaskMonitor> m_task_monitors;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
    };

    /** Stores the order log properties used by Spire Classic. */
    struct OrderLogProperties {

      /** Specifies the type of order status filter used. */
      enum class OrderStatusFilterType {

        /** Lists all orders. */
        ALL_ORDERS,

        /** Lists only live orders. */
        LIVE_ORDERS,

        /** Lists only terminal orders. */
        TERMINAL_ORDERS,

        /** Uses the custom order status filter. */
        CUSTOM
      };

      /** The type of order status filter used. */
      OrderStatusFilterType m_order_status_filter_type;

      /** The custom order status filter. */
      Beam::EnumSet<Nexus::OrderStatus> m_order_status_filter;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
    };

    /** Stores a blotter's properties used by Spire Classic. */
    struct Properties {

      /** The name of the blotter. */
      std::string m_name;

      /** The executing account. */
      Beam::DirectoryEntry m_account;

      /** Whether the blotter is consolidated. */
      bool m_is_consolidated;

      /** The names of the linked blotters. */
      std::vector<std::string> m_links;

      /** The task properties. */
      TaskProperties m_task_properties;

      /** The order log properties. */
      OrderLogProperties m_order_log_properties;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
    };

    /** The saved blotters. */
    std::vector<Properties> m_blotters;

    /** The name of the active blotter. */
    std::string m_active_blotter;

    /** The default task properties. */
    TaskProperties m_default_task_properties;

    template<Beam::IsShuttle S>
    void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void LegacyBlotterSettings::TaskMonitor::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("monitor", m_monitor);
  }

  template<Beam::IsShuttle S>
  void LegacyBlotterSettings::TaskProperties::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("task_monitors", m_task_monitors);
  }

  template<Beam::IsShuttle S>
  void LegacyBlotterSettings::OrderLogProperties::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("order_status_filter_type", m_order_status_filter_type);
    shuttle.shuttle("order_status_filter", m_order_status_filter);
  }

  template<Beam::IsShuttle S>
  void LegacyBlotterSettings::Properties::shuttle(
      S& shuttle, unsigned int version) {
    shuttle.shuttle("name", m_name);
    shuttle.shuttle("account", m_account);
    shuttle.shuttle("is_consolidated", m_is_consolidated);
    shuttle.shuttle("links", m_links);
    shuttle.shuttle("task_properties", m_task_properties);
    shuttle.shuttle("order_log_properties", m_order_log_properties);
  }

  template<Beam::IsShuttle S>
  void LegacyBlotterSettings::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("blotters", m_blotters);
    shuttle.shuttle("active_blotter", m_active_blotter);
    shuttle.shuttle("default_task_properties", m_default_task_properties);
  }
}

#endif
