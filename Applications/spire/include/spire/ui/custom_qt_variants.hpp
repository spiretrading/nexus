#ifndef SPIRE_CUSTOM_VARIANTS_HPP
#define SPIRE_CUSTOM_VARIANTS_HPP
#include <boost/any.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVariant>
#include <Beam/Tasks/Task.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "spire/spire/spire.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  /*! Wraps a MarketCode so that it can be displayed in a model. */
  struct market_token {

    //! Wraps the MarketCode.
    Nexus::MarketCode m_code;

    //! Constructs a default token.
    market_token() = default;

    //! Wraps a MarketCode.
    /*!
      \param code The MarketCode to wrap.
    */
    market_token(Nexus::MarketCode code);
  };

  /*! Wraps a Side so that it can be displayed within the context of a
      position.
   */
  struct position_side_token {

    //! Wraps the Side.
    Nexus::Side m_side;

    //! Constructs a default token.
    position_side_token() = default;

    //! Wraps a Side.
    /*!
      \param side The Side to wrap.
    */
    position_side_token(Nexus::Side side);

    //! Returns the string representation of this Side.
    QString to_string() const;
  };
}

Q_DECLARE_METATYPE(Beam::Tasks::Task::State);
Q_DECLARE_METATYPE(boost::any);
Q_DECLARE_METATYPE(boost::posix_time::ptime);
Q_DECLARE_METATYPE(boost::posix_time::time_duration);
Q_DECLARE_METATYPE(Nexus::CurrencyId);
Q_DECLARE_METATYPE(Nexus::Money);
Q_DECLARE_METATYPE(Nexus::OrderStatus);
Q_DECLARE_METATYPE(Nexus::OrderType);
Q_DECLARE_METATYPE(Nexus::Quantity);
Q_DECLARE_METATYPE(Nexus::Security);
Q_DECLARE_METATYPE(Nexus::Side);
Q_DECLARE_METATYPE(Nexus::TimeInForce);
Q_DECLARE_METATYPE(spire::market_token);
Q_DECLARE_METATYPE(spire::position_side_token);

namespace spire {

  //! Converts a posix time duration into a QTime.
  QTime to_qtime(const boost::posix_time::time_duration& time);

  //! Converts a QTime into a posix time duration.
  boost::posix_time::time_duration to_time_duration(const QTime& time);

  //! Converts a QDateTime into a posix timestamp.
  QDateTime to_qdate_time(const boost::posix_time::ptime& time);

  //! Converts a posix timestamp into a QDateTime.
  boost::posix_time::ptime to_ptime(const QDateTime& time);

  //! Registers the custom QVariant types.
  void register_custom_qt_variants();

  /*! Implements Qt's item delegate to support the custom QVariant types. */
  class custom_variant_item_delegate : public QStyledItemDelegate {
    public:

      //! Constructs an item delegate for custom variants.
      /*!
        \param parent The parent object.
      */
      custom_variant_item_delegate(QObject* parent = nullptr);

      QString displayText(const QVariant& value,
        const QLocale& locale) const override;
  };

  /*! Implements Qt's proxy model to support the custom QVariant types. */
  class custom_variant_sort_filter_proxy_model : public QSortFilterProxyModel {
    public:

      //! Constructs a proxy model for custom variants.
      /*!
        \param parent The parent object.
      */
      custom_variant_sort_filter_proxy_model(QObject* parent = nullptr);

    protected:
      bool lessThan(const QModelIndex& left,
        const QModelIndex& right) const override;
  };
}

#endif
