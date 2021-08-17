#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
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
  template<typename T>
  bool compare(const T& left, const T& right, const QModelIndex& leftIndex,
      const QModelIndex& rightIndex) {
    if(left == right) {
      return leftIndex.row() < rightIndex.row();
    } else {
      return left < right;
    }
  }

  template<typename T, typename... U>
  bool is_equal_any(const std::any& left, const std::any& right) {
    if(left.type() == typeid(T)) {
      return std::any_cast<const T&>(left) == std::any_cast<const T&>(right);
    }
    if constexpr(sizeof...(U) == 0) {
      return false;
    } else {
      return is_equal_any<U...>(left, right);
    }
  }
}

MarketToken::MarketToken(MarketCode code)
  : m_code(code) {}

bool MarketToken::operator ==(MarketToken token) const {
  return m_code == token.m_code;
}

bool MarketToken::operator !=(MarketToken token) const {
  return !(*this == token);
}

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

bool PositionSideToken::operator ==(PositionSideToken token) const {
  return m_side == token.m_side;
}

bool PositionSideToken::operator !=(PositionSideToken token) const {
  return !(*this == token);
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

QVariant Spire::to_qvariant(const std::any& value) {
  if(value.type() == typeid(bool)) {
    return QVariant::fromValue(std::any_cast<bool>(value));
  } else if(value.type() == typeid(int)) {
    return QVariant::fromValue(std::any_cast<int>(value));
  } else if(value.type() == typeid(Quantity)) {
    return QVariant::fromValue(std::any_cast<Quantity>(value));
  } else if(value.type() == typeid(double)) {
    return QVariant::fromValue(std::any_cast<double>(value));
  } else if(value.type() == typeid(ptime)) {
    return QVariant::fromValue(std::any_cast<ptime>(value));
  } else if(value.type() == typeid(posix_time::time_duration)) {
    return QVariant::fromValue(std::any_cast<posix_time::time_duration>(value));
  } else if(value.type() == typeid(std::string)) {
    return QVariant::fromValue(QString::fromStdString(
      std::any_cast<std::string>(value)));
  } else if(value.type() == typeid(CurrencyId)) {
    return QVariant::fromValue(std::any_cast<CurrencyId>(value));
  } else if(value.type() == typeid(MarketToken)) {
    return QVariant::fromValue(std::any_cast<MarketToken>(value));
  } else if(value.type() == typeid(Money)) {
    return QVariant::fromValue(std::any_cast<Money>(value));
  } else if(value.type() == typeid(Quantity)) {
    return QVariant::fromValue(std::any_cast<Quantity>(value));
  } else if(value.type() == typeid(Region)) {
    return QVariant::fromValue(std::any_cast<Region>(value));
  } else if(value.type() == typeid(OrderStatus)) {
    return QVariant::fromValue(std::any_cast<OrderStatus>(value));
  } else if(value.type() == typeid(OrderType)) {
    return QVariant::fromValue(std::any_cast<OrderType>(value));
  } else if(value.type() == typeid(PositionSideToken)) {
    return QVariant::fromValue(std::any_cast<PositionSideToken>(value));
  } else if(value.type() == typeid(Security)) {
    return QVariant::fromValue(std::any_cast<Security>(value));
  } else if(value.type() == typeid(Side)) {
    return QVariant::fromValue(std::any_cast<Side>(value));
  } else if(value.type() == typeid(TimeInForce)) {
    return QVariant::fromValue(std::any_cast<TimeInForce>(value));
  } else if(value.type() == typeid(QColor)) {
    return QVariant::fromValue(std::any_cast<QColor>(value));
  } else if(value.type() == typeid(QString)) {
    return QVariant::fromValue(std::any_cast<QString>(value));
  } else if(value.type() == typeid(QKeySequence)) {
    return QVariant::fromValue(std::any_cast<QKeySequence>(value));
  }
  return QVariant();
}

void Spire::register_custom_qt_variants() {}

const QString& Spire::displayText(Nexus::TimeInForce time_in_force) {
  auto type = time_in_force.GetType();
  if(type == TimeInForce::Type::DAY) {
    static const auto value = QObject::tr("DAY");
    return value;
  } else if(type == TimeInForce::Type::FOK) {
    static const auto value = QObject::tr("FOK");
    return value;
  } else if(type == TimeInForce::Type::GTC) {
    static const auto value = QObject::tr("GTC");
    return value;
  } else if(type == TimeInForce::Type::GTD) {
    static const auto value = QObject::tr("GTD");
    return value;
  } else if(type == TimeInForce::Type::GTX) {
    static const auto value = QObject::tr("GTX");
    return value;
  } else if(type == TimeInForce::Type::IOC) {
    static const auto value = QObject::tr("IOC");
    return value;
  } else if(type == TimeInForce::Type::MOC) {
    static const auto value = QObject::tr("MOC");
    return value;
  } else if(type == TimeInForce::Type::OPG) {
    static const auto value = QObject::tr("OPG");
    return value;
  } else {
    static const auto value = QObject::tr("NONE");
    return value;
  }
}

const QString& Spire::displayText(Side side) {
  if(side == Side::ASK) {
    static const auto value = QObject::tr("Ask");
    return value;
  } else if(side == Side::BID) {
    static const auto value = QObject::tr("Bid");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}

const QString& Spire::displayText(OrderStatus status) {
  if(status == OrderStatus::PENDING_NEW) {
    static const auto value = QObject::tr("Pending New");
    return value;
  } else if(status ==  OrderStatus::REJECTED) {
    static const auto value = QObject::tr("Rejected");
    return value;
  } else if(status ==  OrderStatus::NEW) {
    static const auto value = QObject::tr("New");
    return value;
  } else if(status ==  OrderStatus::PARTIALLY_FILLED) {
    static const auto value = QObject::tr("Partially Filled");
    return value;
  } else if(status ==  OrderStatus::EXPIRED) {
    static const auto value = QObject::tr("Expired");
    return value;
  } else if(status ==  OrderStatus::CANCELED) {
    static const auto value = QObject::tr("Canceled");
    return value;
  } else if(status ==  OrderStatus::SUSPENDED) {
    static const auto value = QObject::tr("Suspended");
    return value;
  } else if(status ==  OrderStatus::STOPPED) {
    static const auto value = QObject::tr("Stopped");
    return value;
  } else if(status ==  OrderStatus::FILLED) {
    static const auto value = QObject::tr("Filled");
    return value;
  } else if(status ==  OrderStatus::DONE_FOR_DAY) {
    static const auto value = QObject::tr("Done For Day");
    return value;
  } else if(status ==  OrderStatus::PENDING_CANCEL) {
    static const auto value = QObject::tr("Pending Cancel");
    return value;
  } else if(status == OrderStatus::CANCEL_REJECT) {
    static const auto value = QObject::tr("Cancel Reject");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}

const QString& Spire::displayText(OrderType type) {
  if(type == OrderType::MARKET) {
    static const auto value = QObject::tr("MKT");
    return value;
  } else if(type == OrderType::LIMIT) {
    static const auto value = QObject::tr("LMT");
    return value;
  } else if(type == OrderType::PEGGED) {
    static const auto value = QObject::tr("PEG");
    return value;
  } else if(type == OrderType::STOP) {
    static const auto value = QObject::tr("STP");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}

QString Spire::displayTextAny(const std::any& value) {
  auto translated_value = to_qvariant(value);
  return CustomVariantItemDelegate().displayText(translated_value);
}

bool Spire::is_equal(const std::any& left, const std::any& right) {
  if(left.type() != right.type()) {
    return false;
  }
  return is_equal_any<bool, int, Quantity, double, ptime,
    posix_time::time_duration, std::string, CurrencyId, MarketToken, Money,
    Region, OrderStatus, OrderType, PositionSideToken, Security, Side,
    TimeInForce, QColor, QString>(left, right);
}

CustomVariantItemDelegate::CustomVariantItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent) {}

QString CustomVariantItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  if(value.canConvert<ptime>()) {
    auto time_value = ToLocalTime(value.value<ptime>());
    auto currentTime = ToLocalTime(
      boost::posix_time::second_clock::universal_time());
    if(time_value.date() == currentTime.date()) {
      return QString::fromStdString(to_simple_string(time_value).substr(12));
    } else {
      return QString::fromStdString(to_simple_string(time_value));
    }
  } else if(value.canConvert<posix_time::time_duration>()) {
    return QString::fromStdString(to_simple_string(
      value.value<posix_time::time_duration>()));
  } else if(value.canConvert<CurrencyId>()) {
    auto& entry = GetDefaultCurrencyDatabase().FromId(
      value.value<CurrencyId>());
    return QString::fromStdString(entry.m_code.GetData());
  } else if(value.canConvert<MarketToken>()) {
    auto& entry = GetDefaultMarketDatabase().FromCode(
      value.value<MarketToken>().m_code);
    return QString::fromStdString(entry.m_displayName);
  } else if(value.canConvert<Money>()) {
    return QString::fromStdString(lexical_cast<std::string>(
      value.value<Money>()));
  } else if(value.canConvert<Quantity>()) {
    return locale.toString(static_cast<double>(value.value<Quantity>()));
  } else if(value.canConvert<OrderStatus>()) {
    return Spire::displayText(value.value<OrderStatus>());
  } else if(value.canConvert<OrderType>()) {
    return Spire::displayText(value.value<OrderType>());
  } else if(value.canConvert<PositionSideToken>()) {
    return value.value<PositionSideToken>().to_string();
  } else if(value.canConvert<Region>()) {
    auto region = value.value<Region>();
    if(region.IsGlobal()) {
      return QObject::tr("Global");
    }
    return QString::fromStdString(region.GetName());
  } else if(value.canConvert<Security>()) {
    return QString::fromStdString(ToWildCardString(value.value<Security>(),
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase()));
  } else if(value.canConvert<Side>()) {
    return Spire::displayText(value.value<Side>());
  } else if(value.canConvert<TimeInForce>()) {
    return Spire::displayText(value.value<TimeInForce>().GetType());
  } else if(value.canConvert<std::any>()) {
    auto translated_value = to_qvariant(value.value<std::any>());
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
  if(left_variant.canConvert<std::any>()) {
    left_variant = to_qvariant(left_variant.value<std::any>());
  }
  auto right_variant = sourceModel()->data(right, sortRole());
  if(right_variant.canConvert<std::any>()) {
    right_variant = to_qvariant(right_variant.value<std::any>());
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
    return compare(displayText(left_variant.value<OrderStatus>()),
      displayText(right_variant.value<OrderStatus>()), left, right);
  } else if(left_variant.canConvert<OrderType>()) {
    return compare(displayText(left_variant.value<OrderType>()),
      displayText(right_variant.value<OrderType>()), left, right);
  } else if(left_variant.canConvert<Security>()) {
    return compare(ToString(left_variant.value<Security>(),
      GetDefaultMarketDatabase()), ToString(right_variant.value<Security>(),
      GetDefaultMarketDatabase()), left, right);
  } else if(left_variant.canConvert<Side>()) {
    return compare(displayText(left_variant.value<Side>()),
      displayText(right_variant.value<Side>()), left, right);
  } else if(left_variant.canConvert<TimeInForce>()) {
    return compare(
      displayText(left_variant.value<TimeInForce>().GetType()),
      displayText(right_variant.value<TimeInForce>().GetType()),
      left, right);
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
  } else if(left_variant.canConvert<QKeySequence>()) {
    auto left = left_variant.value<QKeySequence>().toString();
    auto right = right_variant.value<QKeySequence>().toString();
    return left < right;
  }
  if(left_variant == right_variant) {
    return left.row() < right.row();
  }
  return QSortFilterProxyModel::lessThan(left, right);
}
