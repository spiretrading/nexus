#ifndef SPIRE_CUSTOM_VARIANTS_HPP
#define SPIRE_CUSTOM_VARIANTS_HPP
#include <any>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <QDate>
#include <QDateTime>
#include <QLocale>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeAndSale.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Spire/AnyRef.hpp"

namespace Spire {

  /**
   * Wraps a Side so that it can be displayed within the context of a
   * position.
   */
  struct PositionSideToken {

    /** Wraps the Side. */
    Nexus::Side m_side;

    /** Returns the string representation of this Side. */
    QString to_string() const;

    bool operator ==(const PositionSideToken& token) const = default;
  };
}

Q_DECLARE_METATYPE(boost::gregorian::date);
Q_DECLARE_METATYPE(boost::posix_time::ptime);
Q_DECLARE_METATYPE(boost::posix_time::time_duration);
Q_DECLARE_METATYPE(Nexus::CountryCode);
Q_DECLARE_METATYPE(Nexus::CurrencyId);
Q_DECLARE_METATYPE(Nexus::Money);
Q_DECLARE_METATYPE(Nexus::OrderStatus);
Q_DECLARE_METATYPE(Nexus::OrderType);
Q_DECLARE_METATYPE(Nexus::Quantity);
Q_DECLARE_METATYPE(Nexus::Region);
Q_DECLARE_METATYPE(Nexus::Security);
Q_DECLARE_METATYPE(Nexus::Side);
Q_DECLARE_METATYPE(Nexus::TimeAndSale::Condition);
Q_DECLARE_METATYPE(Nexus::TimeInForce);
Q_DECLARE_METATYPE(Nexus::Venue);
Q_DECLARE_METATYPE(Spire::PositionSideToken);
Q_DECLARE_METATYPE(std::any);

namespace Spire {

  /** Converts a posix time duration into a QTime. */
  QTime to_qtime(boost::posix_time::time_duration time);

  /** Converts a QTime into a posix time duration. */
  boost::posix_time::time_duration to_time_duration(const QTime& time);

  /** Converts a QDateTime into a posix timestamp. */
  QDateTime to_qdate_time(boost::posix_time::ptime time);

  /** Converts a posix timestamp into a QDateTime. */
  boost::posix_time::ptime to_ptime(const QDateTime& time);

  /** Converts an std::any to a QVariant. */
  QVariant to_qvariant(const std::any& value);

  /** Registers the custom QVariant types. */
  void register_custom_qt_variants();

  /** Returns the text representation of a bool. */
  QString to_text(bool value, const QLocale& = QLocale());

  /** Returns the text representation of an unsigned int. */
  QString to_text(unsigned int value, const QLocale& = QLocale());

  /** Returns the text representation of an int. */
  QString to_text(int value, const QLocale& = QLocale());

  /** Returns the text representation of a 64-bit unsigned int. */
  QString to_text(std::uint64_t value, const QLocale& = QLocale());

  /** Returns the text representation of a 64-bit int. */
  QString to_text(std::int64_t value, const QLocale& = QLocale());

  /** Returns the text representation of a double. */
  QString to_text(double value, const QLocale& = QLocale());

  /** Returns the text representation of an std::string. */
  QString to_text(const std::string& value, const QLocale& locale = QLocale());

  /** Returns the text representation of a QString. */
  QString to_text(const QString& value, const QLocale& locale = QLocale());

  /** Returns the text representation of a date. */
  QString to_text(
    boost::gregorian::date date, const QLocale& locale = QLocale());

  /** Returns the text representation of a ptime. */
  QString to_text(
    boost::posix_time::ptime time, const QLocale& locale = QLocale());

  /** Returns the text representation of a time_duration. */
  QString to_text(
    boost::posix_time::time_duration time, const QLocale& locale = QLocale());

  /** Returns the text representation of a CountryCode. */
  QString to_text(Nexus::CountryCode code, const QLocale& locale = QLocale());

  /** Returns the text representation of a CurrencyId. */
  QString to_text(
    Nexus::CurrencyId currency, const QLocale& locale = QLocale());

  /** Returns the text representation of a Money value. */
  QString to_text(Nexus::Money value, const QLocale& locale = QLocale());

  /** Returns the text representation of a Quantity value. */
  QString to_text(Nexus::Quantity value, const QLocale& locale = QLocale());

  /** Returns the text representation of a TimeAndSale::Condition. */
  QString to_text(const Nexus::TimeAndSale::Condition& condition,
    const QLocale& locale = QLocale());

  /** Returns the text representation of a TimeInForce. */
  const QString& to_text(Nexus::TimeInForce time_in_force,
    const QLocale& locale = QLocale());

  /** Returns the text representation of a Side. */
  const QString& to_text(Nexus::Side side, const QLocale& locale = QLocale());

  /** Returns the text representation of an OrderStatus. */
  const QString& to_text(
    Nexus::OrderStatus status, const QLocale& locale = QLocale());

  /** Returns the text representation of an OrderType. */
  const QString& to_text(
    Nexus::OrderType type, const QLocale& locale = QLocale());

  /** Returns the text representation of a PositionSideToken. */
  QString to_text(PositionSideToken token, const QLocale& locale = QLocale());

  /** Returns the text representation of a Region. */
  QString to_text(
    const Nexus::Region& region, const QLocale& locale = QLocale());

  /** Returns the text representation of a Security. */
  QString to_text(
    const Nexus::Security& security, const QLocale& locale = QLocale());

  /** Returns the text representation of a Venue. */
  QString to_text(Nexus::Venue venue, const QLocale& locale = QLocale());

  /** Returns the text representation of a keyboard modifier. */
  const QString& to_text(
    Qt::KeyboardModifier modifier, const QLocale& locale = QLocale());

  /** Returns the text representation of a QKeySequence. */
  QString to_text(const QKeySequence& value, const QLocale& locale = QLocale());

  /** Returns the text representation of the value stored within an AnyRef. */
  QString to_text(const AnyRef& value, const QLocale& locale = QLocale());

  /** Returns the text representation of the value stored within an std::any. */
  QString to_text(const std::any& value, const QLocale& locale = QLocale());

  template<typename T>
  QString to_text(const T&, const QLocale& locale = QLocale()) = delete;

  /**
   * Returns <code>true</code> iff the <i>left</i> value comes before the
   * <i>right</i> value in the type's natural ordering.
   */
  bool compare(const AnyRef& left, const AnyRef& right);

  /** Tests if two <code>std::any</code> have equal types and values. */
  bool is_equal(const std::any& left, const std::any& right);

  /**
   * Constructs an instance of the given type from its text representation.
   * @param T The type to construct.
   * @param text The text used to instantiate the object.
   * @returns An initialized optional iff construction was successful and the
   *          object is valid.
   */
  template<typename T>
  boost::optional<T> from_text(const QString& text) = delete;

  template<>
  boost::optional<QString> from_text(const QString& text);

  template<>
  boost::optional<int> from_text(const QString& text);

  template<>
  boost::optional<double> from_text(const QString& text);

  template<>
  boost::optional<boost::gregorian::date> from_text(const QString& text);

  template<>
  boost::optional<boost::posix_time::ptime> from_text(const QString& text);

  template<>
  boost::optional<boost::posix_time::time_duration>
    from_text(const QString& text);

  template<>
  boost::optional<std::string> from_text(const QString& text);

  template<>
  boost::optional<Nexus::CurrencyId> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Money> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Quantity> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Region> from_text(const QString& text);

  template<>
  boost::optional<Nexus::OrderStatus> from_text(const QString& text);

  template<>
  boost::optional<Nexus::OrderType> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Security> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Side> from_text(const QString& text);

  template<>
  boost::optional<Nexus::TimeInForce> from_text(const QString& text);

  template<>
  boost::optional<Nexus::Venue> from_text(const QString& text);

  template<>
  boost::optional<QColor> from_text(const QString& text);

  template<>
  boost::optional<QKeySequence> from_text(const QString& text);
}

#endif
