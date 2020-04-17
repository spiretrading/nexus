#include "Spire/UI/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/lexical_cast.hpp>
#include <QDateTime>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Nexus/ServiceClients/VirtualServiceClients.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::UI;
using namespace std;

namespace {
  QVariant AnyToVariant(const any& value) {
    if(value.type() == typeid(bool)) {
      return QVariant::fromValue(any_cast<bool>(value));
    } else if(value.type() == typeid(Quantity)) {
      return QVariant::fromValue(any_cast<Quantity>(value));
    } else if(value.type() == typeid(double)) {
      return QVariant::fromValue(any_cast<double>(value));
    } else if(value.type() == typeid(ptime)) {
      return QVariant::fromValue(any_cast<ptime>(value));
    } else if(value.type() == typeid(posix_time::time_duration)) {
      return QVariant::fromValue(any_cast<posix_time::time_duration>(value));
    } else if(value.type() == typeid(string)) {
      return QVariant::fromValue(QString::fromStdString(
        any_cast<string>(value)));
    } else if(value.type() == typeid(CurrencyId)) {
      return QVariant::fromValue(any_cast<CurrencyId>(value));
    } else if(value.type() == typeid(MarketToken)) {
      return QVariant::fromValue(any_cast<MarketToken>(value));
    } else if(value.type() == typeid(Money)) {
      return QVariant::fromValue(any_cast<Money>(value));
    } else if(value.type() == typeid(Quantity)) {
      return QVariant::fromValue(any_cast<Quantity>(value));
    } else if(value.type() == typeid(OrderStatus)) {
      return QVariant::fromValue(any_cast<OrderStatus>(value));
    } else if(value.type() == typeid(Task::State)) {
      return QVariant::fromValue(any_cast<Task::State>(value));
    } else if(value.type() == typeid(OrderType)) {
      return QVariant::fromValue(any_cast<OrderType>(value));
    } else if(value.type() == typeid(PositionSideToken)) {
      return QVariant::fromValue(any_cast<PositionSideToken>(value));
    } else if(value.type() == typeid(Security)) {
      return QVariant::fromValue(any_cast<Security>(value));
    } else if(value.type() == typeid(Side)) {
      return QVariant::fromValue(any_cast<Side>(value));
    } else if(value.type() == typeid(TimeInForce)) {
      return QVariant::fromValue(any_cast<TimeInForce>(value));
    }
    return QString("N/A");
  }

  template<typename T>
  bool Compare(const T& left, const T& right, const QModelIndex& leftIndex,
      const QModelIndex& rightIndex) {
    if(left == right) {
      return leftIndex.row() < rightIndex.row();
    } else {
      return left < right;
    }
  }
}

MarketToken::MarketToken() {}

MarketToken::MarketToken(MarketCode code)
    : m_code(code) {}

PositionSideToken::PositionSideToken() {}

PositionSideToken::PositionSideToken(Side side)
    : m_side(side) {}

QString PositionSideToken::ToString() const {
  if(m_side == Side::BID) {
    return QObject::tr("Long");
  } else if(m_side == Side::ASK) {
    return QObject::tr("Short");
  }
  return QObject::tr("Flat");
}

QTime Spire::UI::ToQTime(const posix_time::time_duration& time) {
  QTime timestamp(static_cast<int>(time.hours()),
    static_cast<int>(time.minutes()), static_cast<int>(time.seconds()),
    static_cast<int>(time.fractional_seconds()) / 1000);
  return timestamp;
}

posix_time::time_duration Spire::UI::ToPosixTimeDuration(const QTime& time) {
  posix_time::time_duration timestamp(time.hour(), time.minute(), time.second(),
    time.msec());
  return timestamp;
}

QDateTime Spire::UI::ToQDateTime(const ptime& time) {
  QDateTime dateTime(QDate(time.date().year(), time.date().month(),
    time.date().day()), ToQTime(time.time_of_day()));
  return dateTime;
}

posix_time::ptime Spire::UI::ToPosixTime(const QDateTime& time) {
  ptime posixTime(gregorian::date(time.date().year(), time.date().month(),
    time.date().day()), posix_time::time_duration(time.time().hour(),
    time.time().minute(), time.time().second(), time.time().msec()));
  return posixTime;
}

void Spire::UI::RegisterCustomQtVariants() {}

CustomVariantItemDelegate::CustomVariantItemDelegate(
    Ref<UserProfile> userProfile, QObject* parent)
    : QStyledItemDelegate(parent),
      m_userProfile(userProfile.Get()) {}

CustomVariantItemDelegate::~CustomVariantItemDelegate() {}

QString CustomVariantItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
   if(value.canConvert<ptime>()) {
    ptime timeValue = ToLocalTime(value.value<ptime>());
    string a = to_simple_string(value.value<ptime>());
    string b = to_simple_string(timeValue);
    auto currentTime = ToLocalTime(
      m_userProfile->GetServiceClients().GetTimeClient().GetTime());
    if(timeValue.date() == currentTime.date()) {
      return QString::fromStdString(to_simple_string(timeValue).substr(12));
    } else {
      return QString::fromStdString(to_simple_string(timeValue));
    }
  } else if(value.canConvert<CurrencyId>()) {
    const CurrencyDatabase::Entry& entry =
      m_userProfile->GetCurrencyDatabase().FromId(value.value<CurrencyId>());
    return QString::fromStdString(entry.m_code.GetData());
  } else if(value.canConvert<MarketToken>()) {
    const MarketDatabase::Entry& entry =
      m_userProfile->GetMarketDatabase().FromCode(
      value.value<MarketToken>().m_code);
    return QString::fromStdString(entry.m_displayName);
  } else if(value.canConvert<Money>()) {
    return QString::fromStdString(value.value<Money>().ToString());
  } else if(value.canConvert<Quantity>()) {
    return QString::fromStdString(
      lexical_cast<std::string>(value.value<Quantity>()));
  } else if(value.canConvert<OrderStatus>()) {

    // TOSTRING TODO
    return QString::fromStdString(ToString(value.value<OrderStatus>()));
  } else if(value.userType() == QMetaTypeId<Task::State>::qt_metatype_id()) {
    return QString::fromStdString(lexical_cast<string>(
      value.value<Task::State>()));
  } else if(value.canConvert<OrderType>()) {

    // TOSTRING TODO
    return QString::fromStdString(ToString(value.value<OrderType>()));
  } else if(value.canConvert<PositionSideToken>()) {
    return value.value<PositionSideToken>().ToString();
  } else if(value.canConvert<Security>()) {
    return QString::fromStdString(ToWildCardString(value.value<Security>(),
      m_userProfile->GetMarketDatabase(), m_userProfile->GetCountryDatabase()));
  } else if(value.canConvert<Side>()) {

    // TOSTRING TODO
    return QString::fromStdString(ToString(value.value<Side>()));
  } else if(value.canConvert<TimeInForce>()) {
    return QString::fromStdString(
      ToString(value.value<TimeInForce>().GetType()));
  } else if(value.canConvert<any>()) {
    QVariant translatedValue = AnyToVariant(value.value<any>());
    return displayText(translatedValue, locale);
  }
  return QStyledItemDelegate::displayText(value, locale);
}

CustomVariantSortFilterProxyModel::CustomVariantSortFilterProxyModel(
    Ref<UserProfile> userProfile, QObject* parent)
    : QSortFilterProxyModel(parent),
      m_userProfile(userProfile.Get()) {
  setDynamicSortFilter(true);
}

CustomVariantSortFilterProxyModel::~CustomVariantSortFilterProxyModel() {}

bool CustomVariantSortFilterProxyModel::lessThan(const QModelIndex& left,
    const QModelIndex& right) const {
  QVariant leftVariant = sourceModel()->data(left, sortRole());
  if(leftVariant.canConvert<any>()) {
    leftVariant = AnyToVariant(leftVariant.value<any>());
  }
  QVariant rightVariant = sourceModel()->data(right, sortRole());
  if(rightVariant.canConvert<any>()) {
    rightVariant = AnyToVariant(rightVariant.value<any>());
  }
  if(leftVariant.userType() != rightVariant.userType()) {
    return QSortFilterProxyModel::lessThan(left, right);
  }
  if(leftVariant.userType() == QMetaTypeId<Task::State>::qt_metatype_id()) {
    return Compare(lexical_cast<string>(leftVariant.value<Task::State>()),
      lexical_cast<string>(rightVariant.value<Task::State>()), left, right);
  } else if(leftVariant.canConvert<ptime>()) {
    return Compare(leftVariant.value<ptime>(), rightVariant.value<ptime>(),
      left, right);
  } else if(leftVariant.canConvert<posix_time::time_duration>()) {
    return Compare(leftVariant.value<posix_time::time_duration>(),
      rightVariant.value<posix_time::time_duration>(), left, right);
  } else if(leftVariant.canConvert<Money>()) {
    return Compare(leftVariant.value<Money>(), rightVariant.value<Money>(),
      left, right);
  } else if(leftVariant.canConvert<Quantity>()) {
    return Compare(leftVariant.value<Quantity>(),
      rightVariant.value<Quantity>(), left, right);
  } else if(leftVariant.canConvert<OrderStatus>()) {

    // TOSTRING TODO
    return Compare(ToString(leftVariant.value<OrderStatus>()),
      ToString(rightVariant.value<OrderStatus>()), left, right);
  } else if(leftVariant.canConvert<OrderType>()) {

    // TOSTRING TODO
    return Compare(ToString(leftVariant.value<OrderType>()),
      ToString(rightVariant.value<OrderType>()), left, right);
  } else if(leftVariant.canConvert<Security>()) {
    return Compare(ToString(leftVariant.value<Security>(),
      m_userProfile->GetMarketDatabase()),
      ToString(rightVariant.value<Security>(),
      m_userProfile->GetMarketDatabase()), left, right);
  } else if(leftVariant.canConvert<Side>()) {

    // TOSTRING TODO
    return Compare(ToString(leftVariant.value<Side>()),
      ToString(rightVariant.value<Side>()), left, right);
  } else if(leftVariant.canConvert<TimeInForce>()) {
    return Compare(ToString(leftVariant.value<TimeInForce>().GetType()),
      ToString(rightVariant.value<TimeInForce>().GetType()), left, right);
  } else if(leftVariant.canConvert<CurrencyId>()) {
    const CurrencyDatabase::Entry& leftEntry =
      m_userProfile->GetCurrencyDatabase().FromId(
      leftVariant.value<CurrencyId>());
    const CurrencyDatabase::Entry& rightEntry =
      m_userProfile->GetCurrencyDatabase().FromId(
      rightVariant.value<CurrencyId>());
    return leftEntry.m_code < rightEntry.m_code;
  } else if(leftVariant.canConvert<PositionSideToken>()) {
    return Compare(leftVariant.value<PositionSideToken>().ToString(),
      rightVariant.value<PositionSideToken>().ToString(), left, right);
  } else if(leftVariant.canConvert<MarketToken>()) {
    const MarketDatabase::Entry& leftEntry =
      m_userProfile->GetMarketDatabase().FromCode(
      leftVariant.value<MarketToken>().m_code);
    const MarketDatabase::Entry& rightEntry =
      m_userProfile->GetMarketDatabase().FromCode(
      rightVariant.value<MarketToken>().m_code);
    return leftEntry.m_displayName < rightEntry.m_displayName;
  }
  if(leftVariant == rightVariant) {
    return left.row() < right.row();
  }
  return QSortFilterProxyModel::lessThan(left, right);
}
