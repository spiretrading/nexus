#ifndef NEXUS_ADMINISTRATION_SERVICE_NOTIFICATION_HPP
#define NEXUS_ADMINISTRATION_SERVICE_NOTIFICATION_HPP
#include <ostream>
#include <sstream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /** Stores a notification sent to an account. */
  struct Notification {

    /** The type used to uniquely identify a notification. */
    using Id = std::string;

    /** Filters notifications by read state. */
    enum class ReadState {

      /** Matches unread notifications. */
      UNREAD = 1,

      /** Matches read notifications. */
      READ = 2,

      /** Matches all notifications. */
      ALL = 3
    };

    /** Lists the categories of notifications. */
    enum class Category {

      /** The notification pertains to an account modification. */
      ACCOUNT_MODIFICATION,

      /** The notification pertains to a report. */
      REPORT
    };

    /** The id that uniquely identifies this notification. */
    Id m_id;

    /** The account that the notification is for. */
    Beam::DirectoryEntry m_account;

    /** The description of the notification. */
    std::string m_description;

    /** Arbitrary data associated with the notification. */
    std::string m_data;

    /** The category of the notification. */
    Category m_category;

    /** The timestamp when the notification was created. */
    boost::posix_time::ptime m_timestamp;

    /** Whether the notification has been read. */
    bool m_is_read;
  };

  /**
   * Makes a Notification for an entitlement modification.
   * @param id The unique identifier for the notification.
   * @param account The account whose entitlements were modified.
   * @param request_id The modification request id.
   * @param status The status of the modification request.
   * @param timestamp The timestamp of the notification.
   * @return The constructed Notification.
   */
  inline Notification make_entitlement_modification_notification(
      Notification::Id id, Beam::DirectoryEntry account,
      AccountModificationRequest::Id request_id,
      AccountModificationRequest::Status status,
      boost::posix_time::ptime timestamp) {
    auto description = [&] {
      if(status == AccountModificationRequest::Status::GRANTED) {
        return std::string("Entitlements have been updated.");
      }
      return std::string("Entitlement modification request has been rejected.");
    }();
    auto data = [&] {
      auto ss = std::ostringstream();
      ss << "{\"request_id\":" << request_id << ",\"status\":\"";
      ss << status << "\"}";
      return ss.str();
    }();
    return Notification(std::move(id), std::move(account),
      std::move(description), std::move(data),
      Notification::Category::ACCOUNT_MODIFICATION, timestamp, false);
  }

  /**
   * Makes a Notification for a risk modification.
   * @param id The unique identifier for the notification.
   * @param account The account whose risk parameters were modified.
   * @param request_id The modification request id.
   * @param status The status of the modification request.
   * @param timestamp The timestamp of the notification.
   * @return The constructed Notification.
   */
  inline Notification make_risk_modification_notification(
      Notification::Id id, Beam::DirectoryEntry account,
      AccountModificationRequest::Id request_id,
      AccountModificationRequest::Status status,
      boost::posix_time::ptime timestamp) {
    auto description = [&] {
      if(status == AccountModificationRequest::Status::GRANTED) {
        return std::string("Risk parameters have been updated.");
      }
      return std::string("Risk modification request has been rejected.");
    }();
    auto data = [&] {
      auto ss = std::ostringstream();
      ss << "{\"request_id\":" << request_id << ",\"status\":\"";
      ss << status << "\"}";
      return ss.str();
    }();
    return Notification(std::move(id), std::move(account),
      std::move(description), std::move(data),
      Notification::Category::ACCOUNT_MODIFICATION, timestamp, false);
  }
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::Notification> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::Notification& value,
        unsigned int version) const {
      shuttle.shuttle("id", value.m_id);
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("description", value.m_description);
      shuttle.shuttle("data", value.m_data);
      shuttle.shuttle("category", value.m_category);
      shuttle.shuttle("timestamp", value.m_timestamp);
      shuttle.shuttle("is_read", value.m_is_read);
    }
  };
}

template<>
class std::formatter<Nexus::Notification::Category> :
    public std::formatter<std::string_view> {
  public:
    auto format(Nexus::Notification::Category value, auto& context) const {
      auto text = [&] {
        if(value == Nexus::Notification::Category::ACCOUNT_MODIFICATION) {
          return "ACCOUNT_MODIFICATION";
        } else if(value == Nexus::Notification::Category::REPORT) {
          return "REPORT";
        }
        return "UNKNOWN";
      }();
      return std::formatter<std::string_view>::format(text, context);
    }
};

template<>
class std::formatter<Nexus::Notification> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::Notification& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(), "({} {} \"{}\" \"{}\" {} {} {})",
          value.m_id, value.m_account, value.m_description, value.m_data,
          value.m_category, Beam::to_string(value.m_timestamp),
          value.m_is_read), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, Notification::Category category) {
    return out << std::format(out.getloc(), "{}", category);
  }

  inline std::ostream& operator <<(
      std::ostream& out, const Notification& notification) {
    return out << std::format(out.getloc(), "{}", notification);
  }
}

#endif
