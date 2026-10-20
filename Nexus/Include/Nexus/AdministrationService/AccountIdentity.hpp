#ifndef NEXUS_ACCOUNT_IDENTITY_HPP
#define NEXUS_ACCOUNT_IDENTITY_HPP
#include <ostream>
#include <string>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Country.hpp"

namespace Nexus {

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
    Beam::SharedBuffer m_photo_id;

    /** Some additional/misc. user notes. */
    std::string m_user_notes;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Nexus::AccountIdentity> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Nexus::AccountIdentity& value,
        unsigned int version) const {
      shuttle.shuttle("registration_time", value.m_registration_time);
      shuttle.shuttle("last_login_time", value.m_last_login_time);
      shuttle.shuttle("first_name", value.m_first_name);
      shuttle.shuttle("last_name", value.m_last_name);
      shuttle.shuttle("e_mail", value.m_email_address);
      shuttle.shuttle("address_line_one", value.m_address_line_one);
      shuttle.shuttle("address_line_two", value.m_address_line_two);
      shuttle.shuttle("address_line_three", value.m_address_line_three);
      shuttle.shuttle("city", value.m_city);
      shuttle.shuttle("province", value.m_province);
      shuttle.shuttle("country", value.m_country);
      shuttle.shuttle("photo_id", value.m_photo_id);
      shuttle.shuttle("user_notes", value.m_user_notes);
    }
  };
}

template<>
class std::formatter<Nexus::AccountIdentity> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Nexus::AccountIdentity& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format(context.locale(),
          "(\"{}\" \"{}\" \"{}\" \"{}\" \"{}\" \"{}\" \"{}\" \"{}\" "
          "\"{}\" \"{}\" {} \"{}\")",
          Beam::to_string(value.m_registration_time),
          Beam::to_string(value.m_last_login_time), value.m_first_name,
          value.m_last_name, value.m_email_address, value.m_address_line_one,
          value.m_address_line_two, value.m_address_line_three, value.m_city,
          value.m_province, value.m_country, value.m_user_notes), context);
    }
};

namespace Nexus {
  inline std::ostream& operator <<(
      std::ostream& out, const AccountIdentity& identity) {
    auto database = static_cast<const CountryDatabase*>(
      out.pword(Beam::ScopedStreamManipulator<CountryDatabase>::ID));
    if(database) {
      return out <<
        std::format(make_locale(out.getloc(), *database), "{}", identity);
    }
    return out << std::format(out.getloc(), "{}", identity);
  }
}

#endif
