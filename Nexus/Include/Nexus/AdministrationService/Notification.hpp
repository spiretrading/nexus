#ifndef NEXUS_ADMINISTRATION_SERVICE_NOTIFICATION_HPP
#define NEXUS_ADMINISTRATION_SERVICE_NOTIFICATION_HPP
#include <ostream>
#include <string>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

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

    /** The category of the notification. */
    Category m_category;

    /** The timestamp when the notification was created. */
    boost::posix_time::ptime m_timestamp;

    /** Whether the notification has been read. */
    bool m_is_read;
  };

  inline std::ostream& operator <<(
      std::ostream& out, Notification::Category category) {
    if(category == Notification::Category::ACCOUNT_MODIFICATION) {
      return out << "ACCOUNT_MODIFICATION";
    } else if(category == Notification::Category::REPORT) {
      return out << "REPORT";
    } else {
      return out << "UNKNOWN";
    }
  }

  inline std::ostream& operator <<(
      std::ostream& out, const Notification& notification) {
    return out << '(' << notification.m_id << ' ' << notification.m_account <<
      " \"" << notification.m_description << "\" " << notification.m_category <<
      ' ' << notification.m_timestamp << ' ' << notification.m_is_read << ')';
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
      shuttle.shuttle("category", value.m_category);
      shuttle.shuttle("timestamp", value.m_timestamp);
      shuttle.shuttle("is_read", value.m_is_read);
    }
  };
}

#endif
