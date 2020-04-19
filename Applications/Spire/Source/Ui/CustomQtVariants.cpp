#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <Beam/TimeService/VirtualTimeClient.hpp>
#include <QDateTime>
#include "Nexus/Definitions/SecuritySet.hpp"

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  QVariant to_variant(const any& value) {
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
    } else if(value.type() == typeid(std::string)) {
      return QVariant::fromValue(QString::fromStdString(
        any_cast<std::string>(value)));
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
  bool compare(const T& left, const T& right, const QModelIndex& leftIndex,
      const QModelIndex& rightIndex) {
    if(left == right) {
      return leftIndex.row() < rightIndex.row();
    } else {
      return left < right;
    }
  }
}

MarketToken::MarketToken(MarketCode code)
    : m_code(code) {}

PositionSideToken::PositionSideToken(Side side)
    : m_side(side) {}

QString PositionSideToken::to_string() const {
  if(m_side == Side::BID) {
    return QObject::tr("Long");
  } else if(m_side == Side::ASK) {
    return QObject::tr("Short");
  }
  return QObject::tr("Flat");
}

QTime Spire::to_qtime(const posix_time::time_duration& time) {
  auto timestamp = QTime(static_cast<int>(time.hours()),
    static_cast<int>(time.minutes()), static_cast<int>(time.seconds()),
    static_cast<int>(time.fractional_seconds() / 1000));
  return timestamp;
}

posix_time::time_duration Spire::to_time_duration(const QTime& time) {
  auto timestamp = posix_time::time_duration(time.hour(), time.minute(),
    time.second(), time.msec());
  return timestamp;
}

QDateTime Spire::to_qdate_time(const ptime& time) {
  auto date_time = QDateTime(QDate(time.date().year(), time.date().month(),
    time.date().day()), to_qtime(time.time_of_day()));
  return date_time;
}

posix_time::ptime Spire::to_ptime(const QDateTime& time) {
  auto posix_time = ptime(gregorian::date(
    time.date().year(), time.date().month(), time.date().day()),
    posix_time::time_duration(time.time().hour(), time.time().minute(),
    time.time().second(), time.time().msec()));
  return posix_time;
}

void Spire::register_custom_qt_variants() {}

CustomVariantItemDelegate::CustomVariantItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

QString CustomVariantItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  if(value.canConvert<ptime>()) {
    auto time_value = ToLocalTime(value.value<ptime>());
    auto a = to_simple_string(value.value<ptime>());
    auto b = to_simple_string(time_value);
    auto currentTime = ToLocalTime(
      boost::posix_time::second_clock::universal_time());
    if(time_value.date() == currentTime.date()) {
      return QString::fromStdString(to_simple_string(time_value).substr(12));
    } else {
      return QString::fromStdString(to_simple_string(time_value));
    }
  } else if(value.canConvert<CurrencyId>()) {
    auto& entry = GetDefaultCurrencyDatabase().FromId(
      value.value<CurrencyId>());
    return QString::fromStdString(entry.m_code.GetData());
  } else if(value.canConvert<MarketToken>()) {
    auto& entry = GetDefaultMarketDatabase().FromCode(
      value.value<MarketToken>().m_code);
    return QString::fromStdString(entry.m_displayName);
  } else if(value.canConvert<Money>()) {
    return QString::fromStdString(value.value<Money>().ToString());
  } else if(value.canConvert<Quantity>()) {
    return locale.toString(static_cast<double>(value.value<Quantity>()));
  } else if(value.canConvert<OrderStatus>()) {
    return QString::fromStdString(ToString(value.value<OrderStatus>()));
  } else if(value.canConvert<OrderType>()) {
    return QString::fromStdString(ToString(value.value<OrderType>()));
  } else if(value.canConvert<PositionSideToken>()) {
    return value.value<PositionSideToken>().to_string();
  } else if(value.canConvert<Security>()) {
    return QString::fromStdString(ToWildCardString(value.value<Security>(),
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()));
  } else if(value.canConvert<Side>()) {
    return QString::fromStdString(ToString(value.value<Side>()));
  } else if(value.canConvert<TimeInForce>()) {
    return QString::fromStdString(
      ToString(value.value<TimeInForce>().GetType()));
  } else if(value.canConvert<any>()) {
    auto translated_value = to_variant(value.value<any>());
    return displayText(translated_value, locale);
  }
  return QStyledItemDelegate::displayText(value, locale);
}

CustomVariantSortFilterProxyModel::CustomVariantSortFilterProxyModel(
    QObject* parent)
    : QSortFilterProxyModel(parent) {
  setDynamicSortFilter(true);
}

bool CustomVariantSortFilterProxyModel::lessThan(const QModelIndex& left,
    const QModelIndex& right) const {
  auto left_variant = sourceModel()->data(left, sortRole());
  if(left_variant.canConvert<any>()) {
    left_variant = to_variant(left_variant.value<any>());
  }
  auto right_variant = sourceModel()->data(right, sortRole());
  if(right_variant.canConvert<any>()) {
    right_variant = to_variant(right_variant.value<any>());
  }
  if(left_variant.type() != right_variant.type()) {
    return QSortFilterProxyModel::lessThan(left, right);
  }
  if(left_variant.canConvert<ptime>()) {
    return compare(left_variant.value<ptime>(), right_variant.value<ptime>(),
      left, right);
  } else if(left_variant.canConvert<posix_time::time_duration>()) {
    return compare(left_variant.value<posix_time::time_duration>(),
      right_variant.value<posix_time::time_duration>(), left, right);
  } else if(left_variant.canConvert<Money>()) {
    return compare(left_variant.value<Money>(), right_variant.value<Money>(),
      left, right);
  } else if(left_variant.canConvert<Quantity>()) {
    return compare(left_variant.value<Quantity>(),
      right_variant.value<Quantity>(), left, right);
  } else if(left_variant.canConvert<OrderStatus>()) {
    return compare(ToString(left_variant.value<OrderStatus>()),
      ToString(right_variant.value<OrderStatus>()), left, right);
  } else if(left_variant.canConvert<OrderType>()) {
    return compare(ToString(left_variant.value<OrderType>()),
      ToString(right_variant.value<OrderType>()), left, right);
  } else if(left_variant.canConvert<Security>()) {
    return compare(ToString(left_variant.value<Security>(),
      GetDefaultMarketDatabase()), ToString(right_variant.value<Security>(),
      GetDefaultMarketDatabase()), left, right);
  } else if(left_variant.canConvert<Side>()) {
    return compare(ToString(left_variant.value<Side>()),
      ToString(right_variant.value<Side>()), left, right);
  } else if(left_variant.canConvert<TimeInForce>()) {
    return compare(ToString(left_variant.value<TimeInForce>().GetType()),
      ToString(right_variant.value<TimeInForce>().GetType()), left, right);
  } else if(left_variant.canConvert<CurrencyId>()) {
    auto& leftEntry = GetDefaultCurrencyDatabase().FromId(
      left_variant.value<CurrencyId>());
    auto& rightEntry = GetDefaultCurrencyDatabase().FromId(
      right_variant.value<CurrencyId>());
    return leftEntry.m_code < rightEntry.m_code;
  } else if(left_variant.canConvert<PositionSideToken>()) {
    return compare(left_variant.value<PositionSideToken>().to_string(),
      right_variant.value<PositionSideToken>().to_string(), left, right);
  } else if(left_variant.canConvert<MarketToken>()) {
    auto& leftEntry = GetDefaultMarketDatabase().FromCode(
      left_variant.value<MarketToken>().m_code);
    auto& rightEntry = GetDefaultMarketDatabase().FromCode(
      right_variant.value<MarketToken>().m_code);
    return leftEntry.m_displayName < rightEntry.m_displayName;
  }
  if(left_variant == right_variant) {
    return left.row() < right.row();
  }
  return QSortFilterProxyModel::lessThan(left, right);
}
