#ifndef SPIRE_CUSTOM_VARIANTS_HPP
#define SPIRE_CUSTOM_VARIANTS_HPP
#include <any>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Spire/AnyRef.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Wraps a MarketCode so that it can be displayed in a model. **/
  struct MarketToken {

    /** The MarketCode being wrapped. */
    Nexus::MarketCode m_code;

    /** Constructs a default token. */
    MarketToken() = default;

    /**
     * Wraps a MarketCode.
     * @param code The MarketCode to wrap.
     */
    MarketToken(Nexus::MarketCode code);

    auto operator <=>(const MarketToken& token) const = default;
  };

  /** Returns a CountryCode's three letter code. */
  QString to_three_letter_code(Nexus::CountryCode code);

  /** Returns the text representation of an int. */
  QString to_text(int value);

  /** Returns the text representation of an std::string. */
  QString to_text(const std::string& value);

  /** Returns the text representation of a date. */
  QString to_text(boost::gregorian::date date);

  /** Returns the text representation of a ptime. */
  QString to_text(boost::posix_time::ptime time);

  /** Returns the text representation of a time_duration. */
  QString to_text(boost::posix_time::time_duration time);

  /** Returns the text representation of a CountryCode. */
  QString to_text(Nexus::CountryCode code);

  /** Returns the text representation of a CurrencyId. */
  QString to_text(Nexus::CurrencyId currency);

  /** Returns the text representation of a MarketToken. */
  QString to_text(MarketToken market);

  /** Returns the text representation of a Money value. */
  QString to_text(Nexus::Money value);

  /** Returns the text representation of a Quantity value. */
  QString to_text(Nexus::Quantity value);

  /** Returns the text representation of a TimeInForce. */
  const QString& to_text(Nexus::TimeInForce time_in_force);

  /** Returns the text representation of a Side. */
  const QString& to_text(Nexus::Side side);

  /** Returns the text representation of an OrderStatus. */
  const QString& to_text(Nexus::OrderStatus status);

  /** Returns the text representation of an OrderType. */
  const QString& to_text(Nexus::OrderType type);

  /** Returns the text representation of a Region. */
  QString to_text(const Nexus::Region& region);

  /** Returns the text representation of a Security. */
  QString to_text(const Nexus::Security& security);

  /** Returns the text representation of a QKeySequence. */
  QString to_text(const QKeySequence& value);

  /** Returns the text representation of the value stored within an AnyRef. */
  QString to_text(const AnyRef& value);

  /** Returns the text representation of the value stored within an std::any. */
  QString to_text(const std::any& value);

  template<typename T>
  QString to_text(const T&) = delete;

  /**
   * Returns <code>true</code> iff the <i>left</i> value comes before the
   * <i>right</i> value in the type's natural ordering.
   */
  bool compare(const AnyRef& left, const AnyRef& right);

  /** Tests if two <code>std::any</code> have equal types and values. */
  bool is_equal(const std::any& left, const std::any& right);

  /**
   * Constructs an instance of the given type from its string representation.
   * @param T The type to construct.
   * @param string The string used to instantiate the object.
   * @returns An initialized optional iff construction was successful and the
   *          object is valid.
   */
  template<typename T>
  boost::optional<T> from_string(const QString& string) = delete;

  template<>
  boost::optional<int> from_string(const QString& string);

  template<>
  boost::optional<double> from_string(const QString& string);

  template<>
  boost::optional<boost::gregorian::date> from_string(const QString& string);

  template<>
  boost::optional<boost::posix_time::ptime> from_string(const QString& string);

  template<>
  boost::optional<boost::posix_time::time_duration>
    from_string(const QString& string);

  template<>
  boost::optional<std::string> from_string(const QString& string);

  template<>
  boost::optional<Nexus::CurrencyId> from_string(const QString& string);

  template<>
  boost::optional<Nexus::Money> from_string(const QString& string);

  template<>
  boost::optional<Nexus::Quantity> from_string(const QString& string);

  template<>
  boost::optional<Nexus::Region> from_string(const QString& string);

  template<>
  boost::optional<Nexus::OrderStatus> from_string(const QString& string);

  template<>
  boost::optional<Nexus::OrderType> from_string(const QString& string);

  template<>
  boost::optional<Nexus::Security> from_string(const QString& string);

  template<>
  boost::optional<Nexus::Side> from_string(const QString& string);

  template<>
  boost::optional<Nexus::TimeInForce> from_string(const QString& string);

  template<>
  boost::optional<QColor> from_string(const QString& string);

  template<>
  boost::optional<QKeySequence> from_string(const QString& string);
}

#endif
