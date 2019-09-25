#ifndef NEXUS_DEFAULTTIMEZONEDATABASE_HPP
#define NEXUS_DEFAULTTIMEZONEDATABASE_HPP
#include <sstream>
#include <boost/date_time/local_time/tz_database.hpp>

namespace Nexus {
namespace Details {
  inline std::string BuildDefaultTimeZoneTable() {
    return
      "\"Asia/Hong_Kong\",\"HKT\",\"HKT\",\"\",\"\",\"+08:00:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\"\n"
      "\"Australian_Eastern_Standard_Time\",\"AEST\",\"AEST\",\"AEST\",\"AEST\",\"+10:00:00\",\"+01:00:00\",\"1;0;10\",\"+02:00:00\",\"1;0;4\",\"+03:00:00\"\n"
      "\"Eastern_Time\",\"EST\",\"Eastern Standard Time\",\"EDT\",\"Eastern Daylight Time\",\"-05:00:00\",\"+01:00:00\",\"2;0;3\",\"+02:00:00\",\"1;0;11\",\"+02:00:00\"\n"
      "\"UTC\",\"UTC\",\"UTC\",\"\",\"\",\"+00:00:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\";\n";
  }

  inline boost::local_time::tz_database BuildDefaultTimeZoneDatabase() {
    boost::local_time::tz_database database;
    std::stringstream stream(BuildDefaultTimeZoneTable());
    database.load_from_stream(stream);
    return database;
  }
}

  //! Returns the default time zone table, typically used for testing
  //! purposes.
  inline const std::string& GetDefaultTimeZoneTable() {
    static auto database = Details::BuildDefaultTimeZoneTable();
    return database;
  }

  //! Returns the default time zone database, typically used for testing
  //! purposes.
  inline const boost::local_time::tz_database& GetDefaultTimeZoneDatabase() {
    static auto database = Details::BuildDefaultTimeZoneDatabase();
    return database;
  }
}

#endif
