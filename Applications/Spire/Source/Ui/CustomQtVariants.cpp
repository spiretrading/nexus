#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QStyledItemDelegate>
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
  bool compare(const AnyRef& left, const AnyRef& right) {
    return any_cast<T>(left) < any_cast<T>(right);
  }

  template<typename T>
  bool compare_text(const AnyRef& left, const AnyRef& right) {
    return to_text(any_cast<T>(left)) < to_text(any_cast<T>(right));
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

QString Spire::to_text(int value) {
  return to_text(std::any(value));
}

QString Spire::to_text(const std::string& value) {
  return QString::fromStdString(value);
}

QString Spire::to_text(gregorian::date date) {
  return QString::fromStdString(to_iso_extended_string(date));
}

QString Spire::to_text(ptime time) {
  auto local_time = ToLocalTime(time);
  auto current_time = ToLocalTime(posix_time::second_clock::universal_time());
  if(local_time.date() == current_time.date()) {
    return QString::fromStdString(to_simple_string(local_time).substr(12));
  }
  return QString::fromStdString(to_simple_string(local_time));
}

QString Spire::to_text(posix_time::time_duration time) {
  return QString::fromStdString(to_simple_string(time));
}

QString Spire::to_text(CurrencyId currency) {
  auto& entry = GetDefaultCurrencyDatabase().FromId(currency);
  return QString::fromStdString(entry.m_code.GetData());
}

QString Spire::to_text(MarketToken market) {
  auto& entry = GetDefaultMarketDatabase().FromCode(market.m_code);
  return QString::fromStdString(entry.m_displayName);
}

QString Spire::to_text(Money value) {
  return QString::fromStdString(lexical_cast<std::string>(value));
}

QString Spire::to_text(Quantity value) {
  static auto locale = QLocale();
  return locale.toString(static_cast<double>(value));
}

const QString& Spire::to_text(Nexus::TimeInForce time_in_force) {
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
    static const auto value = QObject::tr("");
    return value;
  }
}

const QString& Spire::to_text(Side side) {
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

const QString& Spire::to_text(OrderStatus status) {
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

const QString& Spire::to_text(OrderType type) {
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
    static const auto value = QObject::tr("");
    return value;
  }
}

QString Spire::to_text(PositionSideToken token) {
  return token.to_string();
}

QString Spire::to_text(const Region& region) {
  if(region.IsGlobal()) {
    return QObject::tr("Global");
  }
  return QString::fromStdString(region.GetName());
}

QString Spire::to_text(const Security& security) {
  return QString::fromStdString(ToWildCardString(
    security, GetDefaultMarketDatabase(), GetDefaultCountryDatabase()));
}

QString Spire::to_text(const QKeySequence& value) {
  return value.toString();
}

QString Spire::to_text(const AnyRef& value) {
  return to_text(to_any(value));
}

QString Spire::to_text(const std::any& value) {
  if(value.type() == typeid(gregorian::date)) {
    return to_text(std::any_cast<gregorian::date>(value));
  } else if(value.type() == typeid(ptime)) {
    return to_text(std::any_cast<ptime>(value));
  } else if(value.type() == typeid(posix_time::time_duration)) {
    return to_text(std::any_cast<posix_time::time_duration>(value));
  } else if(value.type() == typeid(CurrencyId)) {
    return to_text(std::any_cast<CurrencyId>(value));
  } else if(value.type() == typeid(MarketToken)) {
    return to_text(std::any_cast<MarketToken>(value));
  } else if(value.type() == typeid(Money)) {
    return to_text(std::any_cast<Money>(value));
  } else if(value.type() == typeid(Quantity)) {
    return to_text(std::any_cast<Quantity>(value));
  } else if(value.type() == typeid(OrderStatus)) {
    return to_text(std::any_cast<OrderStatus>(value));
  } else if(value.type() == typeid(OrderType)) {
    return to_text(std::any_cast<OrderType>(value));
  } else if(value.type() == typeid(PositionSideToken)) {
    return to_text(std::any_cast<PositionSideToken>(value));
  } else if(value.type() == typeid(Region)) {
    return to_text(std::any_cast<Region>(value));
  } else if(value.type() == typeid(Security)) {
    return to_text(std::any_cast<Security>(value));
  } else if(value.type() == typeid(Side)) {
    return to_text(std::any_cast<Side>(value));
  } else if(value.type() == typeid(TimeInForce)) {
    return to_text(std::any_cast<TimeInForce>(value));
  } else if(value.type() == typeid(std::string)) {
    return QString::fromStdString(std::any_cast<std::string>(value));
  } else if(value.type() == typeid(QString)) {
    return std::any_cast<QString>(value);
  } else if(value.type() == typeid(const char*)) {
    return QString::fromUtf8(std::any_cast<const char*>(value));
  } else {
    static auto delegate = QStyledItemDelegate();
    static auto locale = QLocale();
    if(value.type() == typeid(bool)) {
      return delegate.displayText(std::any_cast<bool>(value), locale);
    } else if(value.type() == typeid(unsigned int)) {
      return delegate.displayText(std::any_cast<unsigned int>(value), locale);
    } else if(value.type() == typeid(int)) {
      return delegate.displayText(std::any_cast<int>(value), locale);
    } else if(value.type() == typeid(std::uint64_t)) {
      return delegate.displayText(std::any_cast<std::uint64_t>(value), locale);
    } else if(value.type() == typeid(std::int64_t)) {
      return delegate.displayText(std::any_cast<std::int64_t>(value), locale);
    } else if(value.type() == typeid(double)) {
      return delegate.displayText(std::any_cast<double>(value), locale);
    }
  }
  return QString();
}

bool Spire::compare(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() != right.get_type()) {
    return false;
  }
  if(left.get_type() == typeid(gregorian::date)) {
    return ::compare<gregorian::date>(left, right);
  } else if(left.get_type() == typeid(ptime)) {
    return ::compare<ptime>(left, right);
  } else if(left.get_type() == typeid(posix_time::time_duration)) {
    return ::compare<posix_time::time_duration>(left, right);
  } else if(left.get_type() == typeid(Money)) {
    return ::compare<Money>(left, right);
  } else if(left.get_type() == typeid(Quantity)) {
    return ::compare<Quantity>(left, right);
  } else if(left.get_type() == typeid(OrderStatus)) {
    return ::compare<OrderStatus>(left, right);
  } else if(left.get_type() == typeid(OrderType)) {
    return ::compare<OrderType>(left, right);
  } else if(left.get_type() == typeid(Security)) {
    return compare_text<Security>(left, right);
  } else if(left.get_type() == typeid(Side)) {
    return compare_text<Side>(left, right);
  } else if(left.get_type() == typeid(TimeInForce)) {
    return compare_text<TimeInForce>(left, right);
  } else if(left.get_type() == typeid(CurrencyId)) {
    return compare_text<CurrencyId>(left, right);
  } else if(left.get_type() == typeid(PositionSideToken)) {
    return compare_text<PositionSideToken>(left, right);
  } else if(left.get_type() == typeid(MarketToken)) {
    return compare_text<MarketToken>(left, right);
  } else if(left.get_type() == typeid(QMetaType::QKeySequence)) {
    return compare_text<QKeySequence>(left, right);
  } else if(left.get_type() == typeid(bool)) {
    return ::compare<bool>(left, right);
  } else if(left.get_type() == typeid(unsigned int)) {
    return ::compare<unsigned int>(left, right);
  } else if(left.get_type() == typeid(int)) {
    return ::compare<int>(left, right);
  } else if(left.get_type() == typeid(std::uint64_t)) {
    return ::compare<std::uint64_t>(left, right);
  } else if(left.get_type() == typeid(std::int64_t)) {
    return ::compare<std::int64_t>(left, right);
  } else if(left.get_type() == typeid(double)) {
    return ::compare<double>(left, right);
  }
  return false;
}

bool Spire::is_equal(const std::any& left, const std::any& right) {
  if(left.type() != right.type()) {
    return false;
  }
  return is_equal_any<bool, int, std::int64_t, std::uint64_t, Quantity, double,
    gregorian::date, ptime, posix_time::time_duration, std::string, CurrencyId,
    MarketToken, Money, Region, OrderStatus, OrderType, PositionSideToken,
    Security, Side, TimeInForce, QColor, QString>(left, right);
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
