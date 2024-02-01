#ifndef SPIRE_CUSTOMVARIANTS_HPP
#define SPIRE_CUSTOMVARIANTS_HPP
#include <boost/any.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QVariant>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Tasks/Task.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/LegacyUI/LegacyUI.hpp"

namespace Spire::LegacyUI {

  /*! \struct MarketToken
      \brief Wraps a MarketCode so that it can be displayed in a model.
   */
  struct MarketToken {

    //! Wraps the MarketCode.
    Nexus::MarketCode m_code;

    //! Constructs a default MarketToken.
    MarketToken();

    //! Wraps a MarketCode.
    /*!
      \param code The MarketCode to wrap.
    */
    MarketToken(Nexus::MarketCode code);
  };

  /*! \struct PositionSideToken
      \brief Wraps a Side so that it can be displayed within the context of a
             position.
   */
  struct PositionSideToken {

    //! Wraps the Side.
    Nexus::Side m_side;

    //! Constructs a default PositionSideToken.
    PositionSideToken();

    //! Wraps a Side.
    /*!
      \param side The Side to wrap.
    */
    PositionSideToken(Nexus::Side side);

    //! Returns the string representation of this Side.
    QString ToString() const;
  };
}

Q_DECLARE_METATYPE(Spire::Task::State);
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
Q_DECLARE_METATYPE(Spire::LegacyUI::MarketToken);
Q_DECLARE_METATYPE(Spire::LegacyUI::PositionSideToken);

namespace Spire::LegacyUI {

  //! Converts a posix time duration into a QTime.
  QTime ToQTime(const boost::posix_time::time_duration& time);

  //! Converts a QTime into a posix time duration.
  boost::posix_time::time_duration ToPosixTimeDuration(const QTime& time);

  //! Converts a QDateTime into a posix timestamp.
  QDateTime ToQDateTime(const boost::posix_time::ptime& time);

  //! Converts a posix timestamp into a QDateTime.
  boost::posix_time::ptime ToPosixTime(const QDateTime& time);

  //! Registers the custom QVariant types.
  void RegisterCustomQtVariants();

  //! Returns the test representation of a Side.
  const QString& displayText(Nexus::Side side);

  //! Returns the test representation of an OrderStatus.
  const QString& displayText(Nexus::OrderStatus status);

  //! Returns the test representation of an OrderType.
  const QString& displayText(Nexus::OrderType type);

  /*! \class CustomVariantItemDelegate
      \brief Extends Qt's item delegate to support the custom QVariant types.
   */
  class CustomVariantItemDelegate : public QStyledItemDelegate {
    public:

      //! Constructs a CustomVariantItemDelegate.
      /*!
        \param userProfile The user's profile.
        \param parent The parent object.
      */
      CustomVariantItemDelegate(Beam::Ref<UserProfile> userProfile,
        QObject* parent = nullptr);

      virtual ~CustomVariantItemDelegate();

      virtual QString displayText(const QVariant& value,
        const QLocale& locale) const;

    private:
      UserProfile* m_userProfile;
  };

  /*! \class CustomVariantSortFilterProxyModel
      \brief Extends Qt's proxy model to support the custom QVariant types.
   */
  class CustomVariantSortFilterProxyModel : public QSortFilterProxyModel {
    public:

      //! Constructs a CustomVariantSortFilterProxyModel.
      /*!
        \param userProfile The user's profile.
        \param parent The parent object.
      */
      CustomVariantSortFilterProxyModel(Beam::Ref<UserProfile> userProfile,
        QObject* parent = nullptr);

      virtual ~CustomVariantSortFilterProxyModel();

    protected:
      virtual bool lessThan(const QModelIndex& left,
        const QModelIndex& right) const;

    private:
      UserProfile* m_userProfile;
  };
}

#endif
