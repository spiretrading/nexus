#ifndef NEXUS_ACCOUNT_IDENTITY_HPP
#define NEXUS_ACCOUNT_IDENTITY_HPP
#include <string>
#include <ostream>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"

namespace Nexus::AdministrationService {

  /** Stores identity related info about an account. */
  struct AccountIdentity {

    /** When the account was registered/created. */
    boost::posix_time::ptime registration_time;

    /** The last time the account logged onto the system. */
    boost::posix_time::ptime last_login_time;

    /** The first name. */
    std::string first_name;

    /** The last name. */
    std::string last_name;

    /** The e-mail address. */
    std::string email_address;

    /** The first line of the account's address, typically the street. */
    std::string address_line_one;

    /** The second line of the account's address, such as floor number. */
    std::string address_line_two;

    /** The third line of the account's address, used for misc. purposes. */
    std::string address_line_three;

    /** The city of residence. */
    std::string city;

    /** The province/state/region of residence. */
    std::string province;

    /** The country of residence. */
    CountryCode country;

    /** The photo ID in JPG format. */
    Beam::IO::SharedBuffer photo_id;

    /** Some additional/misc. user notes. */
    std::string user_notes;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const AccountIdentity& identity) {
    return out << "(\"" << identity.registration_time << "\" " <<
      '"' << identity.last_login_time << "\" " <<
      '"' << identity.first_name << "\" " <<
      '"' << identity.last_name << "\" " <<
      '"' << identity.email_address << "\" " <<
      '"' << identity.address_line_one << "\" " <<
      '"' << identity.address_line_two << "\" " <<
      '"' << identity.address_line_three << "\" " <<
      '"' << identity.city << "\" " <<
      '"' << identity.province << "\" " <<
      identity.country << ' ' <<
      '"' << identity.user_notes << "\")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::AccountIdentity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::AccountIdentity& value,
        unsigned int version) const {
      shuttle.Shuttle("registration_time", value.registration_time);
      shuttle.Shuttle("last_login_time", value.last_login_time);
      shuttle.Shuttle("first_name", value.first_name);
      shuttle.Shuttle("last_name", value.last_name);
      shuttle.Shuttle("e_mail", value.email_address);
      shuttle.Shuttle("address_line_one", value.address_line_one);
      shuttle.Shuttle("address_line_two", value.address_line_two);
      shuttle.Shuttle("address_line_three", value.address_line_three);
      shuttle.Shuttle("city", value.city);
      shuttle.Shuttle("province", value.province);
      shuttle.Shuttle("country", value.country);
      shuttle.Shuttle("photo_id", value.photo_id);
      shuttle.Shuttle("user_notes", value.user_notes);
    }
  };
}

#endif
