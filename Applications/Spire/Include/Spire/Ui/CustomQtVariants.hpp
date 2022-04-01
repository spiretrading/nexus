#ifndef SPIRE_CUSTOM_VARIANTS_HPP
#define SPIRE_CUSTOM_VARIANTS_HPP
#include <any>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVariant>
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
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Wraps a MarketCode so that it can be displayed in a model. **/
  struct MarketToken {

    /** Wraps the MarketCode. */
    Nexus::MarketCode m_code;

    /** Constructs a default token. */
    MarketToken() = default;

    /**
     * Wraps a MarketCode.
     * @param code The MarketCode to wrap.
     */
    MarketToken(Nexus::MarketCode code);

    /**
     * Returns <code>true</code> iff <i>token</i> has the same <i>code</i> as
     * <code>this</code>.
     */
    bool operator ==(MarketToken token) const;

    /**
     * Returns <code>true</code> iff <i>token</i> has a different <i>code</i>
     * from <code>this</code>.
     */
    bool operator !=(MarketToken token) const;
  };

  /**
   * Wraps a Side so that it can be displayed within the context of a
   * position.
   */
  struct PositionSideToken {

    /** Wraps the Side. */
    Nexus::Side m_side;

    /** Constructs a default token. */
    PositionSideToken() = default;

    /**
     * Wraps a Side.
     * @param side The Side to wrap.
     */
    PositionSideToken(Nexus::Side side);

    /** Returns the string representation of this Side. */
    QString to_string() const;

    /**
     * Returns <code>true</code> iff <i>token</i> has the same <i>side</i> as
     * <code>this</code>.
     */
    bool operator ==(PositionSideToken token) const;

    /**
     * Returns <code>true</code> iff <i>token</i> has a different <i>side</i>
     * from <code>this</code>.
     */
    bool operator !=(PositionSideToken token) const;
  };
}

Q_DECLARE_METATYPE(boost::gregorian::date);
Q_DECLARE_METATYPE(boost::posix_time::ptime);
Q_DECLARE_METATYPE(boost::posix_time::time_duration);
Q_DECLARE_METATYPE(Nexus::CurrencyId);
Q_DECLARE_METATYPE(Nexus::Money);
Q_DECLARE_METATYPE(Nexus::OrderStatus);
Q_DECLARE_METATYPE(Nexus::OrderType);
Q_DECLARE_METATYPE(Nexus::Quantity);
Q_DECLARE_METATYPE(Nexus::Region);
Q_DECLARE_METATYPE(Nexus::Security);
Q_DECLARE_METATYPE(Nexus::Side);
Q_DECLARE_METATYPE(Nexus::TimeInForce);
Q_DECLARE_METATYPE(Spire::MarketToken);
Q_DECLARE_METATYPE(Spire::PositionSideToken);

/** Add back this style when charting is implemented. */
//Q_DECLARE_METATYPE(Spire::TrendLineStyle);
Q_DECLARE_METATYPE(std::any);

namespace Spire {

  /** Converts a posix time duration into a QTime. */
  QTime to_qtime(const boost::posix_time::time_duration& time);

  /** Converts a QTime into a posix time duration. */
  boost::posix_time::time_duration to_time_duration(const QTime& time);

  /** Converts a QDateTime into a posix timestamp. */
  QDateTime to_qdate_time(const boost::posix_time::ptime& time);

  /** Converts a posix timestamp into a QDateTime. */
  boost::posix_time::ptime to_ptime(const QDateTime& time);

  /** Converts an std::any to a QVariant. */
  QVariant to_qvariant(const std::any& value);

  /** Registers the custom QVariant types. */
  void register_custom_qt_variants();

  /** Returns the text representation of a TimeInForce. */
  const QString& displayText(Nexus::TimeInForce time_in_force);

  /** Returns the text representation of a Side. */
  const QString& displayText(Nexus::Side side);

  /** Returns the text representation of an OrderStatus. */
  const QString& displayText(Nexus::OrderStatus status);

  /** Returns the text representation of an OrderType. */
  const QString& displayText(Nexus::OrderType type);

  /** Returns the text representation of the value stored within an std::any. */
  QString displayText(const std::any& value);

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

  /** Implements Qt's item delegate to support the custom QVariant types. **/
  class CustomVariantItemDelegate : public QStyledItemDelegate {
    public:

      /**
       * Constructs an item delegate for custom variants.
       * @param parent The parent object.
       */
      explicit CustomVariantItemDelegate(QObject* parent = nullptr);

      QString displayText(const QVariant& value,
        const QLocale& locale = QLocale()) const override;
  };

  /** Implements Qt's proxy model to support the custom QVariant types. **/
  class CustomVariantSortFilterProxyModel : public QSortFilterProxyModel {
    public:

      /**
       * Constructs a proxy model for custom variants.
       * @param parent The parent object.
       */
      explicit CustomVariantSortFilterProxyModel(QObject* parent = nullptr);

    protected:
      bool lessThan(const QModelIndex& left,
        const QModelIndex& right) const override;
  };

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
