#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QDateTime>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/AnyRef.hpp"

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

  template<typename T>
  optional<T> from_string_lexical_cast(const QString& string) {
    try {
      return lexical_cast<T>(string.toStdString());
    } catch(const bad_lexical_cast&) {
      return none;
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

QTime Spire::to_qtime(posix_time::time_duration time) {
  return QTime(static_cast<int>(time.hours()),
    static_cast<int>(time.minutes()), static_cast<int>(time.seconds()),
    static_cast<int>(time.fractional_seconds() / 1000));
}

posix_time::time_duration Spire::to_time_duration(const QTime& time) {
  return posix_time::time_duration(
    time.hour(), time.minute(), time.second(), time.msec());
}

QDateTime Spire::to_qdate_time(ptime time) {
  return QDateTime(QDate(time.date().year(), time.date().month(),
    time.date().day()), to_qtime(time.time_of_day()));
}

ptime Spire::to_ptime(const QDateTime& time) {
  return ptime(
    gregorian::date(time.date().year(), time.date().month(), time.date().day()),
      posix_time::time_duration(time.time().hour(), time.time().minute(),
        time.time().second(), time.time().msec()));
}

QVariant Spire::to_qvariant(const std::any& value) {
  if(value.type() == typeid(bool)) {
    return QVariant::fromValue(std::any_cast<bool>(value));
  } else if(value.type() == typeid(int)) {
    return QVariant::fromValue(std::any_cast<int>(value));
  } else if(value.type() == typeid(double)) {
    return QVariant::fromValue(std::any_cast<double>(value));
  } else if(value.type() == typeid(gregorian::date)) {
    return QVariant::fromValue(std::any_cast<gregorian::date>(value));
  } else if(value.type() == typeid(ptime)) {
    return QVariant::fromValue(std::any_cast<ptime>(value));
  } else if(value.type() == typeid(posix_time::time_duration)) {
    return QVariant::fromValue(std::any_cast<posix_time::time_duration>(value));
  } else if(value.type() == typeid(std::string)) {
    return QVariant::fromValue(
      QString::fromStdString(std::any_cast<std::string>(value)));
  } else if(value.type() == typeid(CountryCode)) {
    return QVariant::fromValue(std::any_cast<CountryCode>(value));
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
  } else if(value.type() == typeid(const char*)) {
    return QVariant::fromValue(
      QString::fromUtf8(std::any_cast<const char*>(value)));
  }
  return QVariant();
}

void Spire::register_custom_qt_variants() {}

QString Spire::displayText(int value) {
  return displayText(std::any(value));
}

QString Spire::displayText(const std::string& value) {
  return QString::fromStdString(value);
}

QString Spire::displayText(gregorian::date date) {
  return QString::fromStdString(to_iso_extended_string(date));
}

QString Spire::displayText(ptime time) {
  auto local_time = ToLocalTime(time);
  auto current_time = ToLocalTime(posix_time::second_clock::universal_time());
  if(local_time.date() == current_time.date()) {
    return QString::fromStdString(to_simple_string(local_time).substr(12));
  }
  return QString::fromStdString(to_simple_string(local_time));
}

QString Spire::displayText(posix_time::time_duration time) {
  return QString::fromStdString(to_simple_string(time));
}

QString Spire::displayText(CountryCode code) {
  auto& entry = GetDefaultCountryDatabase().FromCode(code);
  return QString::fromStdString(entry.m_threeLetterCode.GetData());
}

QString Spire::displayText(CurrencyId currency) {
  auto& entry = GetDefaultCurrencyDatabase().FromId(currency);
  return QString::fromStdString(entry.m_code.GetData());
}

QString Spire::displayText(MarketToken market) {
  auto& entry = GetDefaultMarketDatabase().FromCode(market.m_code);
  return QString::fromStdString(entry.m_displayName);
}

QString Spire::displayText(Money value) {
  return QString::fromStdString(lexical_cast<std::string>(value));
}

QString Spire::displayText(Quantity value) {
  static auto locale = QLocale();
  return locale.toString(static_cast<double>(value));
}

const QString& Spire::displayText(Nexus::TimeInForce time_in_force) {
  auto type = time_in_force.GetType();
  if(type == TimeInForce::Type::DAY) {
    static const auto value = QObject::tr("Day");
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
    static const auto value = QObject::tr("Sell");
    return value;
  } else if(side == Side::BID) {
    static const auto value = QObject::tr("Buy");
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
  } else if(status == OrderStatus::REJECTED) {
    static const auto value = QObject::tr("Rejected");
    return value;
  } else if(status == OrderStatus::NEW) {
    static const auto value = QObject::tr("New");
    return value;
  } else if(status == OrderStatus::PARTIALLY_FILLED) {
    static const auto value = QObject::tr("Partially Filled");
    return value;
  } else if(status == OrderStatus::EXPIRED) {
    static const auto value = QObject::tr("Expired");
    return value;
  } else if(status == OrderStatus::CANCELED) {
    static const auto value = QObject::tr("Canceled");
    return value;
  } else if(status == OrderStatus::SUSPENDED) {
    static const auto value = QObject::tr("Suspended");
    return value;
  } else if(status == OrderStatus::STOPPED) {
    static const auto value = QObject::tr("Stopped");
    return value;
  } else if(status == OrderStatus::FILLED) {
    static const auto value = QObject::tr("Filled");
    return value;
  } else if(status == OrderStatus::DONE_FOR_DAY) {
    static const auto value = QObject::tr("Done For Day");
    return value;
  } else if(status == OrderStatus::PENDING_CANCEL) {
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
    static const auto value = QObject::tr("Market");
    return value;
  } else if(type == OrderType::LIMIT) {
    static const auto value = QObject::tr("Limit");
    return value;
  } else if(type == OrderType::PEGGED) {
    static const auto value = QObject::tr("Pegged");
    return value;
  } else if(type == OrderType::STOP) {
    static const auto value = QObject::tr("Stop");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}

QString Spire::displayText(PositionSideToken token) {
  return token.to_string();
}

QString Spire::displayText(const Region& region) {
  if(region.IsGlobal()) {
    return QObject::tr("Global");
  }
  if(region.GetSecurities().size() == 1 && region.GetMarkets().empty() &&
      region.GetCountries().empty()) {
    return displayText(*region.GetSecurities().begin());
  } else if(region.GetMarkets().size() == 1 && region.GetSecurities().empty() &&
      region.GetCountries().empty()) {
    return displayText(MarketToken(*region.GetMarkets().begin()));
  } else if(region.GetCountries().size() == 1 &&
      region.GetSecurities().empty() && region.GetMarkets().empty()) {
    return displayText(*region.GetCountries().begin());
  }
  return QString::fromStdString(region.GetName());
}

QString Spire::displayText(const Security& security) {
  return QString::fromStdString(ToWildCardString(
    security, GetDefaultMarketDatabase(), GetDefaultCountryDatabase()));
}

QString Spire::displayText(const AnyRef& value) {
  return displayText(to_any(value));
}

QString Spire::displayText(const std::any& value) {
  auto translated_value = to_qvariant(value);
  return CustomVariantItemDelegate().displayText(translated_value);
}

bool Spire::is_equal(const std::any& left, const std::any& right) {
  if(left.type() != right.type()) {
    return false;
  }
  return is_equal_any<bool, int, Quantity, double, gregorian::date, ptime,
    posix_time::time_duration, std::string, CountryCode, CurrencyId,
    MarketToken, Money, Region, OrderStatus, OrderType, PositionSideToken,
    Security, Side, TimeInForce, QColor, QString>(left, right);
}

CustomVariantItemDelegate::CustomVariantItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent) {}

QString CustomVariantItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  if(value.type() == QVariant::Type::UserType) {
    if(value.canConvert<gregorian::date>()) {
      return Spire::displayText(value.value<gregorian::date>());
    } else if(value.canConvert<ptime>()) {
      return Spire::displayText(value.value<ptime>());
    } else if(value.canConvert<posix_time::time_duration>()) {
      return Spire::displayText(value.value<posix_time::time_duration>());
    } else if(value.canConvert<CountryCode>()) {
      return Spire::displayText(value.value<CountryCode>());
    } else if(value.canConvert<CurrencyId>()) {
      return Spire::displayText(value.value<CurrencyId>());
    } else if(value.canConvert<MarketToken>()) {
      return Spire::displayText(value.value<MarketToken>());
    } else if(value.canConvert<Money>()) {
      return Spire::displayText(value.value<Money>());
    } else if(value.canConvert<Quantity>()) {
      return Spire::displayText(value.value<Quantity>());
    } else if(value.canConvert<OrderStatus>()) {
      return Spire::displayText(value.value<OrderStatus>());
    } else if(value.canConvert<OrderType>()) {
      return Spire::displayText(value.value<OrderType>());
    } else if(value.canConvert<PositionSideToken>()) {
      return Spire::displayText(value.value<PositionSideToken>());
    } else if(value.canConvert<Region>()) {
      return Spire::displayText(value.value<Region>());
    } else if(value.canConvert<Security>()) {
      return Spire::displayText(value.value<Security>());
    } else if(value.canConvert<Side>()) {
      return Spire::displayText(value.value<Side>());
    } else if(value.canConvert<TimeInForce>()) {
      return Spire::displayText(value.value<TimeInForce>());
    } else if(value.canConvert<std::any>()) {
      auto translated_value = to_qvariant(value.value<std::any>());
      return displayText(translated_value, locale);
    }
  } else if(value.type() == QMetaType::QKeySequence) {
    return value.value<QKeySequence>().toString();
  } else if(value.type() == QMetaType::QString) {
    return value.value<QString>();
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
  if(left_variant.canConvert<gregorian::date>()) {
    return compare(left_variant.value<gregorian::date>(),
      right_variant.value<gregorian::date>(), left, right);
  } else if(left_variant.canConvert<ptime>()) {
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
    return compare(displayText(left_variant.value<TimeInForce>()),
      displayText(right_variant.value<TimeInForce>()), left, right);
  } else if(left_variant.canConvert<CountryCode>()) {
    auto& leftEntry = GetDefaultCountryDatabase().FromCode(
      left_variant.value<CountryCode>());
    auto& rightEntry = GetDefaultCountryDatabase().FromCode(
      right_variant.value<CountryCode>());
    return leftEntry.m_code < rightEntry.m_code;
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
  } else if(left_variant.type() == QMetaType::QKeySequence) {
    auto left = left_variant.value<QKeySequence>().toString();
    auto right = right_variant.value<QKeySequence>().toString();
    return left < right;
  }
  if(left_variant == right_variant) {
    return left.row() < right.row();
  }
  return QSortFilterProxyModel::lessThan(left, right);
}

template<>
optional<int> Spire::from_string(const QString& string) {
  return from_string_lexical_cast<int>(string);
}

template<>
optional<double> Spire::from_string(const QString& string) {
  return from_string_lexical_cast<double>(string);
}

template<>
optional<gregorian::date> Spire::from_string(const QString& string) {
  return from_string_lexical_cast<gregorian::date>(string);
}

template<>
optional<ptime> Spire::from_string(const QString& string) {
  return from_string_lexical_cast<ptime>(string);
}

template<>
optional<posix_time::time_duration> Spire::from_string(const QString& string) {
  return from_string_lexical_cast<posix_time::time_duration>(string);
}

template<>
optional<std::string> Spire::from_string(const QString& string) {
  return string.toStdString();
}

template<>
optional<CurrencyId> Spire::from_string(const QString& string) {
  if(auto id = ParseCurrency(string.toStdString());
      id != CurrencyId::NONE) {
    return id;
  }
  return none;
}

template<>
optional<Money> Spire::from_string(const QString& string) {
  return Money::FromValue(string.toStdString());
}

template<>
optional<Quantity> Spire::from_string(const QString& string) {
  return Quantity::FromValue(string.toStdString());
}

template<>
optional<Region> Spire::from_string(const QString& string) {
  return Region(string.toStdString());
}

template<>
optional<OrderStatus> Spire::from_string(const QString& string) {
  if(string == QObject::tr("Pending New")) {
    return optional<OrderStatus>(OrderStatus::PENDING_NEW);
  } else if(string == QObject::tr("Rejected")) {
    return optional<OrderStatus>(OrderStatus::REJECTED);
  } else if(string == QObject::tr("New")) {
    return optional<OrderStatus>(OrderStatus::NEW);
  } else if(string == QObject::tr("Partially Filled")) {
    return optional<OrderStatus>(OrderStatus::PARTIALLY_FILLED);
  } else if(string == QObject::tr("Expired")) {
    return optional<OrderStatus>(OrderStatus::EXPIRED);
  } else if(string == QObject::tr("Canceled")) {
    return optional<OrderStatus>(OrderStatus::CANCELED);
  } else if(string == QObject::tr("Suspended")) {
    return optional<OrderStatus>(OrderStatus::SUSPENDED);
  } else if(string == QObject::tr("Stopped")) {
    return optional<OrderStatus>(OrderStatus::STOPPED);
  } else if(string == QObject::tr("Filled")) {
    return optional<OrderStatus>(OrderStatus::FILLED);
  } else if(string == QObject::tr("Done For Day")) {
    return optional<OrderStatus>(OrderStatus::DONE_FOR_DAY);
  } else if(string == QObject::tr("Pending Cancel")) {
    return optional<OrderStatus>(OrderStatus::PENDING_CANCEL);
  } else if(string == QObject::tr("Cancel Reject")) {
    return optional<OrderStatus>(OrderStatus::CANCEL_REJECT);
  } else if(string == QObject::tr("None")) {
    return optional<OrderStatus>(OrderStatus::NONE);
  }
  return none;
}

template<>
optional<OrderType> Spire::from_string(const QString& string) {
  if(string == QObject::tr("Market")) {
    return optional<OrderType>(OrderType::MARKET);
  } else if(string == QObject::tr("Limit")) {
    return optional<OrderType>(OrderType::LIMIT);
  } else if(string == QObject::tr("Pegged")) {
    return optional<OrderType>(OrderType::PEGGED);
  } else if(string == QObject::tr("Stop")) {
    return optional<OrderType>(OrderType::STOP);
  } else if(string == QObject::tr("None")) {
    return optional<OrderType>(OrderType::NONE);
  }
  return none;
}

template<>
optional<Security> Spire::from_string(const QString& string) {
  if(auto security = ParseSecurity(string.toStdString());
      security != Security()) {
    return security;
  }
  return none;
}

template<>
optional<Side> Spire::from_string(const QString& string) {
  if(string == QObject::tr("Sell")) {
    return optional<Side>(Side::ASK);
  } else if(string == QObject::tr("Buy")) {
    return optional<Side>(Side::BID);
  } else if(string == QObject::tr("None")) {
    return optional<Side>(Side::NONE);
  }
  return none;
}

template<>
optional<TimeInForce> Spire::from_string(const QString& string) {
  if(string == QObject::tr("DAY")) {
    return optional<TimeInForce>(TimeInForce::Type::DAY);
  } else if(string == QObject::tr("FOK")) {
    return optional<TimeInForce>(TimeInForce::Type::FOK);
  } else if(string == QObject::tr("GTC")) {
    return optional<TimeInForce>(TimeInForce::Type::GTC);
  } else if(string == QObject::tr("GTD")) {
    return optional<TimeInForce>(TimeInForce::Type::GTD);
  } else if(string == QObject::tr("GTX")) {
    return optional<TimeInForce>(TimeInForce::Type::GTX);
  } else if(string == QObject::tr("IOC")) {
    return optional<TimeInForce>(TimeInForce::Type::IOC);
  } else if(string == QObject::tr("MOC")) {
    return optional<TimeInForce>(TimeInForce::Type::MOC);
  } else if(string == QObject::tr("OPG")) {
    return optional<TimeInForce>(TimeInForce::Type::OPG);
  } else if(string == QObject::tr("NONE")) {
    return optional<TimeInForce>(TimeInForce::Type::NONE);
  }
  return none;
}

template<>
optional<QColor> Spire::from_string(const QString& string) {
  if(auto color = QColor(string); color.isValid()) {
    return color;
  }
  return none;
}

template<>
optional<QKeySequence> Spire::from_string(const QString& string) {
  if(auto sequence = QKeySequence(string); !sequence.isEmpty()) {
    return sequence;
  }
  return none;
}
