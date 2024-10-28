#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <QStyledItemDelegate>

using namespace Beam;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::date_time;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  decltype(auto) type_of(const AnyRef& arg) {
    return arg.get_type();
  }

  decltype(auto) type_of(const std::any& arg) {
    return arg.type();
  }

  template<typename T>
  decltype(auto) cast_any(const std::any& a) {
    return std::any_cast<const T&>(a);
  }

  template<typename T>
  decltype(auto) cast_any(const AnyRef& a) {
    return any_cast<T>(a);
  }

  template<typename T, typename... U>
  struct apply_any {
    template<typename Any, typename F>
    auto operator ()(const Any& arg, const F& f) const {
      if(type_of(arg) == typeid(T)) {
        return f(cast_any<T>(arg));
      }
      if constexpr(sizeof...(U) == 0) {
        return false;
      } else {
        return apply_any<U...>()(arg, f);
      }
    }

    template<typename Any, typename F>
    auto operator ()(const Any& arg1, const AnyRef& arg2, const F& f) const {
      if(type_of(arg1) == typeid(T)) {
        return f(cast_any<T>(arg1), cast_any<T>(arg2));
      }
      if constexpr(sizeof...(U) == 0) {
        return false;
      } else {
        return apply_any<U...>()(arg1, arg2, f);
      }
    }
  };

  template<typename... T>
  bool compare_any(const AnyRef& left, const AnyRef& right) {
    return apply_any<T...>()(left, right,
      [&] (const auto& left, const auto& right) {
        return left < right;
      });
  }

  template<typename T>
  bool compare_text(const AnyRef& left, const AnyRef& right) {
    return to_text(any_cast<T>(left)) < to_text(any_cast<T>(right));
  }

  template<typename... T>
  bool is_equal_any(const std::any& left, const std::any& right) {
    return apply_any<T...>()(left, right,
      [&] (const auto& left, const auto& right) {
        return left == right;
      });
  }

  template<typename T>
  optional<T> from_string_lexical_cast(const QString& string) {
    try {
      return lexical_cast<T>(string.toStdString());
    } catch(const bad_lexical_cast&) {
      return none;
    }
  }

  auto& get_style_delegate() {
    static auto delegate = QStyledItemDelegate();
    return delegate;
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
    gregorian::date(static_cast<unsigned short>(time.date().year()),
      static_cast<unsigned short>(time.date().month()),
      static_cast<unsigned short>(time.date().day())),
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
  } else if(value.type() == typeid(TimeAndSale::Condition)) {
    return QVariant::fromValue(std::any_cast<TimeAndSale::Condition>(value));
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

QString Spire::to_text(bool value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(unsigned int value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(int value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(std::uint64_t value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(std::int64_t value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(double value, const QLocale& locale) {
  return get_style_delegate().displayText(value, locale);
}

QString Spire::to_text(const std::string& value, const QLocale& locale) {
  return QString::fromStdString(value);
}

QString Spire::to_text(const QString& value, const QLocale& locale) {
  return value;
}

QString Spire::to_text(gregorian::date date, const QLocale& locale) {
  return QString::fromStdString(to_iso_extended_string(date));
}

QString Spire::to_text(ptime time, const QLocale& locale) {
  auto local_time = ToLocalTime(time);
  auto current_time = ToLocalTime(posix_time::second_clock::universal_time());
  if(local_time.date() == current_time.date()) {
    return QString::fromStdString(to_simple_string(local_time).substr(12));
  }
  return QString::fromStdString(to_simple_string(local_time));
}

QString Spire::to_text(posix_time::time_duration time, const QLocale& locale) {
  return QString::fromStdString(to_simple_string(time));
}

QString Spire::to_text(CountryCode code, const QLocale& locale) {
  auto& entry = GetDefaultCountryDatabase().FromCode(code);
  return QString::fromStdString(entry.m_threeLetterCode.GetData());
}

QString Spire::to_text(CurrencyId currency, const QLocale& locale) {
  auto& entry = GetDefaultCurrencyDatabase().FromId(currency);
  return QString::fromStdString(entry.m_code.GetData());
}

QString Spire::to_text(MarketToken market, const QLocale& locale) {
  auto& entry = GetDefaultMarketDatabase().FromCode(market.m_code);
  return QString::fromStdString(entry.m_displayName);
}

QString Spire::to_text(Money value, const QLocale& locale) {
  return QString::fromStdString(lexical_cast<std::string>(value));
}

QString Spire::to_text(Quantity value, const QLocale& locale) {
  return locale.toString(static_cast<double>(value));
}

QString Spire::to_text(
    const TimeAndSale::Condition& condition, const QLocale& locale) {
  return QString::fromStdString(condition.m_code);
}

const QString& Spire::to_text(
    Nexus::TimeInForce time_in_force, const QLocale& locale) {
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

const QString& Spire::to_text(Side side, const QLocale& locale) {
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

const QString& Spire::to_text(OrderStatus status, const QLocale& locale) {
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

const QString& Spire::to_text(OrderType type, const QLocale& locale) {
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

QString Spire::to_text(PositionSideToken token, const QLocale& locale) {
  return token.to_string();
}

QString Spire::to_text(const Region& region, const QLocale& locale) {
  if(region.IsGlobal()) {
    return QObject::tr("Global");
  }
  if(region.GetSecurities().size() == 1 && region.GetMarkets().empty() &&
      region.GetCountries().empty()) {
    return to_text(*region.GetSecurities().begin(), locale);
  } else if(region.GetMarkets().size() == 1 && region.GetSecurities().empty() &&
      region.GetCountries().empty()) {
    return to_text(MarketToken(*region.GetMarkets().begin()), locale);
  } else if(region.GetCountries().size() == 1 &&
      region.GetSecurities().empty() && region.GetMarkets().empty()) {
    return to_text(*region.GetCountries().begin(), locale);
  }
  return QString::fromStdString(region.GetName());
}

QString Spire::to_text(const Security& security, const QLocale& locale) {
  return QString::fromStdString(ToString(security, GetDefaultMarketDatabase()));
}

const QString& Spire::to_text(
    Qt::KeyboardModifier modifier, const QLocale& locale) {
  if(modifier == Qt::NoModifier) {
    static const auto value = QObject::tr("Default");
    return value;
  } else if(modifier == Qt::ShiftModifier) {
    static const auto value = QObject::tr("Shift");
    return value;
  } else if(modifier == Qt::ControlModifier) {
    static const auto value = QObject::tr("Ctrl");
    return value;
  } else if(modifier == Qt::AltModifier) {
    static const auto value = QObject::tr("Alt");
    return value;
  } else {
    static const auto value = QObject::tr("None");
    return value;
  }
}

QString Spire::to_text(const QKeySequence& value, const QLocale& locale) {
  return value.toString();
}

QString Spire::to_text(const AnyRef& value, const QLocale& locale) {
  return to_text(to_any(value), locale);
}

QString Spire::to_text(const std::any& value, const QLocale& locale) {
  if(value.type() == typeid(gregorian::date)) {
    return to_text(std::any_cast<gregorian::date>(value), locale);
  } else if(value.type() == typeid(ptime)) {
    return to_text(std::any_cast<ptime>(value), locale);
  } else if(value.type() == typeid(posix_time::time_duration)) {
    return to_text(std::any_cast<posix_time::time_duration>(value), locale);
  } else if(value.type() == typeid(CountryCode)) {
    return to_text(std::any_cast<CountryCode>(value), locale);
  } else if(value.type() == typeid(CurrencyId)) {
    return to_text(std::any_cast<CurrencyId>(value), locale);
  } else if(value.type() == typeid(MarketToken)) {
    return to_text(std::any_cast<MarketToken>(value), locale);
  } else if(value.type() == typeid(Money)) {
    return to_text(std::any_cast<Money>(value), locale);
  } else if(value.type() == typeid(Quantity)) {
    return to_text(std::any_cast<Quantity>(value), locale);
  } else if(value.type() == typeid(OrderStatus)) {
    return to_text(std::any_cast<OrderStatus>(value), locale);
  } else if(value.type() == typeid(OrderType)) {
    return to_text(std::any_cast<OrderType>(value), locale);
  } else if(value.type() == typeid(PositionSideToken)) {
    return to_text(std::any_cast<PositionSideToken>(value), locale);
  } else if(value.type() == typeid(Region)) {
    return to_text(std::any_cast<Region>(value), locale);
  } else if(value.type() == typeid(Security)) {
    return to_text(std::any_cast<Security>(value), locale);
  } else if(value.type() == typeid(Side)) {
    return to_text(std::any_cast<Side>(value), locale);
  } else if(value.type() == typeid(TimeAndSale::Condition)) {
    return to_text(std::any_cast<TimeAndSale::Condition>(value), locale);
  } else if(value.type() == typeid(TimeInForce)) {
    return to_text(std::any_cast<TimeInForce>(value), locale);
  } else if(value.type() == typeid(QKeySequence)) {
    return to_text(std::any_cast<QKeySequence>(value), locale);
  } else if(value.type() == typeid(std::string)) {
    return QString::fromStdString(std::any_cast<std::string>(value));
  } else if(value.type() == typeid(QColor)) {
    return std::any_cast<QColor>(value).name();
  } else if(value.type() == typeid(QString)) {
    return std::any_cast<QString>(value);
  } else if(value.type() == typeid(const char*)) {
    return QString::fromUtf8(std::any_cast<const char*>(value));
  } else if(value.type() == typeid(bool)) {
    return to_text(std::any_cast<bool>(value), locale);
  } else if(value.type() == typeid(unsigned int)) {
    return to_text(std::any_cast<unsigned int>(value), locale);
  } else if(value.type() == typeid(int)) {
    return to_text(std::any_cast<int>(value), locale);
  } else if(value.type() == typeid(std::uint64_t)) {
    return to_text(std::any_cast<std::uint64_t>(value), locale);
  } else if(value.type() == typeid(std::int64_t)) {
    return to_text(std::any_cast<std::int64_t>(value), locale);
  } else if(value.type() == typeid(double)) {
    return to_text(std::any_cast<double>(value), locale);
  }
  return QString();
}

bool Spire::compare(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() != right.get_type()) {
    return false;
  }
  if(left.get_type() == typeid(TimeInForce)) {
    return compare_text<TimeInForce>(left, right);
  }
  return compare_any<bool, int, optional<int>, std::int64_t,
    optional<std::int64_t>, std::uint64_t, optional<std::uint64_t>, Quantity,
    optional<Quantity>, double, optional<double>, gregorian::date, ptime,
    posix_time::time_duration, std::string, CountryCode, CurrencyId, Money,
    optional<Money>, Region, OrderStatus, OrderType, Security, Side,
    QKeySequence, QString>(left, right);
}

bool Spire::is_equal(const std::any& left, const std::any& right) {
  if(left.type() != right.type()) {
    return false;
  }
  return is_equal_any<bool, int, std::int64_t, std::uint64_t, Quantity, double,
    gregorian::date, ptime, posix_time::time_duration, std::string, CountryCode,
    CurrencyId, CurrencyId, MarketToken, Money, Region, OrderStatus, OrderType,
    PositionSideToken, Security, Side, TimeInForce, QColor, QKeySequence,
    QString>(left, right);
}

template<>
optional<QString> Spire::from_text(const QString& text) {
  return text;
}

template<>
optional<int> Spire::from_text(const QString& text) {
  return from_string_lexical_cast<int>(text);
}

template<>
optional<double> Spire::from_text(const QString& text) {
  return from_string_lexical_cast<double>(text);
}

template<>
optional<gregorian::date> Spire::from_text(const QString& text) {
  return from_string_lexical_cast<gregorian::date>(text);
}

template<>
optional<ptime> Spire::from_text(const QString& text) {
  return from_string_lexical_cast<ptime>(text);
}

template<>
optional<posix_time::time_duration> Spire::from_text(const QString& text) {
  return from_string_lexical_cast<posix_time::time_duration>(text);
}

template<>
optional<std::string> Spire::from_text(const QString& text) {
  return text.toStdString();
}

template<>
optional<CurrencyId> Spire::from_text(const QString& text) {
  if(auto id = ParseCurrency(text.toStdString());
      id != CurrencyId::NONE) {
    return id;
  }
  return none;
}

template<>
optional<Money> Spire::from_text(const QString& text) {
  return Money::FromValue(text.toStdString());
}

template<>
optional<Quantity> Spire::from_text(const QString& text) {
  return Quantity::FromValue(text.toStdString());
}

template<>
optional<Region> Spire::from_text(const QString& text) {
  return Region(text.toStdString());
}

template<>
optional<OrderStatus> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Pending New")) {
    return optional<OrderStatus>(OrderStatus::PENDING_NEW);
  } else if(text == QObject::tr("Rejected")) {
    return optional<OrderStatus>(OrderStatus::REJECTED);
  } else if(text == QObject::tr("New")) {
    return optional<OrderStatus>(OrderStatus::NEW);
  } else if(text == QObject::tr("Partially Filled")) {
    return optional<OrderStatus>(OrderStatus::PARTIALLY_FILLED);
  } else if(text == QObject::tr("Expired")) {
    return optional<OrderStatus>(OrderStatus::EXPIRED);
  } else if(text == QObject::tr("Canceled")) {
    return optional<OrderStatus>(OrderStatus::CANCELED);
  } else if(text == QObject::tr("Suspended")) {
    return optional<OrderStatus>(OrderStatus::SUSPENDED);
  } else if(text == QObject::tr("Stopped")) {
    return optional<OrderStatus>(OrderStatus::STOPPED);
  } else if(text == QObject::tr("Filled")) {
    return optional<OrderStatus>(OrderStatus::FILLED);
  } else if(text == QObject::tr("Done For Day")) {
    return optional<OrderStatus>(OrderStatus::DONE_FOR_DAY);
  } else if(text == QObject::tr("Pending Cancel")) {
    return optional<OrderStatus>(OrderStatus::PENDING_CANCEL);
  } else if(text == QObject::tr("Cancel Reject")) {
    return optional<OrderStatus>(OrderStatus::CANCEL_REJECT);
  } else if(text == QObject::tr("None")) {
    return optional<OrderStatus>(OrderStatus::NONE);
  }
  return none;
}

template<>
optional<OrderType> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Market")) {
    return optional<OrderType>(OrderType::MARKET);
  } else if(text == QObject::tr("Limit")) {
    return optional<OrderType>(OrderType::LIMIT);
  } else if(text == QObject::tr("Pegged")) {
    return optional<OrderType>(OrderType::PEGGED);
  } else if(text == QObject::tr("Stop")) {
    return optional<OrderType>(OrderType::STOP);
  } else if(text == QObject::tr("None")) {
    return optional<OrderType>(OrderType::NONE);
  }
  return none;
}

template<>
optional<Security> Spire::from_text(const QString& text) {
  if(auto security = ParseSecurity(text.toStdString());
      security != Security()) {
    return security;
  }
  return none;
}

template<>
optional<Side> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Sell")) {
    return optional<Side>(Side::ASK);
  } else if(text == QObject::tr("Buy")) {
    return optional<Side>(Side::BID);
  } else if(text == QObject::tr("None")) {
    return optional<Side>(Side::NONE);
  }
  return none;
}

template<>
optional<TimeInForce> Spire::from_text(const QString& text) {
  if(text == QObject::tr("DAY")) {
    return optional<TimeInForce>(TimeInForce::Type::DAY);
  } else if(text == QObject::tr("FOK")) {
    return optional<TimeInForce>(TimeInForce::Type::FOK);
  } else if(text == QObject::tr("GTC")) {
    return optional<TimeInForce>(TimeInForce::Type::GTC);
  } else if(text == QObject::tr("GTD")) {
    return optional<TimeInForce>(TimeInForce::Type::GTD);
  } else if(text == QObject::tr("GTX")) {
    return optional<TimeInForce>(TimeInForce::Type::GTX);
  } else if(text == QObject::tr("IOC")) {
    return optional<TimeInForce>(TimeInForce::Type::IOC);
  } else if(text == QObject::tr("MOC")) {
    return optional<TimeInForce>(TimeInForce::Type::MOC);
  } else if(text == QObject::tr("OPG")) {
    return optional<TimeInForce>(TimeInForce::Type::OPG);
  } else if(text == QObject::tr("NONE")) {
    return optional<TimeInForce>(TimeInForce::Type::NONE);
  }
  return none;
}

template<>
optional<QColor> Spire::from_text(const QString& text) {
  if(auto color = QColor(text); color.isValid()) {
    return color;
  }
  return none;
}

template<>
optional<QKeySequence> Spire::from_text(const QString& text) {
  if(auto sequence = QKeySequence(text); !sequence.isEmpty()) {
    return sequence;
  }
  return none;
}
