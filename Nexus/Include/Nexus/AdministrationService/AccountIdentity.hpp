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
    boost::posix_time::ptime m_registration_time;

    /** The last time the account logged onto the system. */
    boost::posix_time::ptime m_last_login_time;

    /** The first name. */
    std::string m_first_name;

    /** The last name. */
    std::string m_last_name;

    /** The e-mail address. */
    std::string m_email_address;

    /** The first line of the account's address, typically the street. */
    std::string m_address_line_one;

    /** The second line of the account's address, such as floor number. */
    std::string m_address_line_two;

    /** The third line of the account's address, used for misc. purposes. */
    std::string m_address_line_three;

    /** The city of residence. */
    std::string m_city;

    /** The province/state/region of residence. */
    std::string m_province;

    /** The country of residence. */
    CountryCode m_country;

    /** The photo ID in JPG format. */
    Beam::IO::SharedBuffer m_photo_id;

    /** Some additional/misc. user notes. */
    std::string m_user_notes;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const AccountIdentity& identity) {
    return out << "(\"" << identity.m_registration_time << "\" " <<
      '"' << identity.m_last_login_time << "\" " <<
      '"' << identity.m_first_name << "\" " <<
      '"' << identity.m_last_name << "\" " <<
      '"' << identity.m_email_address << "\" " <<
      '"' << identity.m_address_line_one << "\" " <<
      '"' << identity.m_address_line_two << "\" " <<
      '"' << identity.m_address_line_three << "\" " <<
      '"' << identity.m_city << "\" " <<
      '"' << identity.m_province << "\" " <<
      identity.m_country << ' ' <<
      '"' << identity.m_user_notes << "\")";
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::AccountIdentity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::AccountIdentity& value,
        unsigned int version) const {
      shuttle.Shuttle("registration_time", value.m_registration_time);
      shuttle.Shuttle("last_login_time", value.m_last_login_time);
      shuttle.Shuttle("first_name", value.m_first_name);
      shuttle.Shuttle("last_name", value.m_last_name);
      shuttle.Shuttle("e_mail", value.m_email_address);
      shuttle.Shuttle("address_line_one", value.m_address_line_one);
      shuttle.Shuttle("address_line_two", value.m_address_line_two);
      shuttle.Shuttle("address_line_three", value.m_address_line_three);
      shuttle.Shuttle("city", value.m_city);
      shuttle.Shuttle("province", value.m_province);
      shuttle.Shuttle("country", value.m_country);
      shuttle.Shuttle("photo_id", value.m_photo_id);
      shuttle.Shuttle("user_notes", value.m_user_notes);
    }
  };
}

#endif
