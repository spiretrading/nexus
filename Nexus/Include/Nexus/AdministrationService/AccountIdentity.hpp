#ifndef NEXUS_ACCOUNT_IDENTITY_HPP
#define NEXUS_ACCOUNT_IDENTITY_HPP
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

  /** Stores identity related info about an account. */
  struct AccountIdentity {

    /** When the account was registered/created. */
    boost::posix_time::ptime m_registrationTime;

    /** The last time the account logged onto the system. */
    boost::posix_time::ptime m_lastLoginTime;

    /** The first name. */
    std::string m_firstName;

    /** The last name. */
    std::string m_lastName;

    /** The e-mail address. */
    std::string m_emailAddress;

    /** The first line of the account's address, typically the street. */
    std::string m_addressLineOne;

    /** The second line of the account's address, such as floor number. */
    std::string m_addressLineTwo;

    /** The third line of the account's address, used for misc. purposes. */
    std::string m_addressLineThree;

    /** The city of residence. */
    std::string m_city;

    /** The province/state/region of residence. */
    std::string m_province;

    /** The country of residence. */
    CountryCode m_country;

    /** The photo ID in JPG format. */
    Beam::IO::SharedBuffer m_photoId;

    /** Some additional/misc. user notes. */
    std::string m_userNotes;
  };
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::AccountIdentity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::AccountIdentity& value,
        unsigned int version) {
      shuttle.Shuttle("registration_time", value.m_registrationTime);
      shuttle.Shuttle("last_login_time", value.m_lastLoginTime);
      shuttle.Shuttle("first_name", value.m_firstName);
      shuttle.Shuttle("last_name", value.m_lastName);
      shuttle.Shuttle("e_mail", value.m_emailAddress);
      shuttle.Shuttle("address_line_one", value.m_addressLineOne);
      shuttle.Shuttle("address_line_two", value.m_addressLineTwo);
      shuttle.Shuttle("address_line_three", value.m_addressLineThree);
      shuttle.Shuttle("city", value.m_city);
      shuttle.Shuttle("province", value.m_province);
      shuttle.Shuttle("country", value.m_country);
      shuttle.Shuttle("photo_id", value.m_photoId);
      shuttle.Shuttle("user_notes", value.m_userNotes);
    }
  };
}

#endif
