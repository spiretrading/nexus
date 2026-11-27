#include "Spire/Ui/CustomQtVariants.hpp"
#include <Beam/TimeService/ToLocalTime.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <QStyledItemDelegate>
#include "Spire/Spire/Hash.hpp"
#include "Spire/Ui/AccountListItem.hpp"
#include "Spire/Ui/DistinctTableAggregator.hpp"

using namespace Beam;
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
  using ComparableTypes = apply_any<int, double, std::int64_t, std::uint64_t,
    unsigned int, float, long double, short, unsigned short, signed char,
    unsigned char, long, unsigned long, long long, unsigned long long,
    optional<int>, optional<double>, optional<std::int64_t>,
    optional<std::uint64_t>, optional<unsigned int>, optional<float>,
    optional<long double>, optional<short>, optional<unsigned short>,
    optional<signed char>, optional<unsigned char>, optional<long>,
    optional<unsigned long>, optional<long long>, optional<unsigned long long>,
    T...>;

  template<typename T>
  QString format_number(const T& value, const QLocale& locale) {
    if constexpr(std::signed_integral<T>) {
      return to_text(static_cast<std::int64_t>(value), locale);
    } else if constexpr(std::unsigned_integral<T>) {
      return to_text(static_cast<std::uint64_t>(value), locale);
    } else if constexpr(std::floating_point<T>) {
      return to_text(static_cast<double>(value), locale);
    } else {
      return to_text(value, locale);
    }
  }

  template<typename T>
  QString format_number(const optional<T>& value, const QLocale& locale) {
    if(value) {
      return format_number(*value, locale);
    }
    return {};
  }

  template<std::floating_point T>
  bool is_nan(const T& value) {
    return std::isnan(value);
  }

  template<std::floating_point T>
  bool is_nan(const optional<T>& value) {
    return value && std::isnan(*value);
  }

  template<typename... T>
  bool compare_any(const AnyRef& left, const AnyRef& right) {
    return ComparableTypes<T...>()(left, right,
      [&] (const auto& left, const auto& right) {
        if constexpr(requires { is_nan(left); }) {
          return !is_nan(left) && (is_nan(right) || left < right);
        }
        return left < right;
      });
  }

  template<typename T>
  bool compare_text(const AnyRef& left, const AnyRef& right) {
    return to_text(any_cast<T>(left)) < to_text(any_cast<T>(right));
  }

  QString find_smallest_text(const Scope& scope, int n) {
    auto texts = std::vector<QString>();
    for(auto& country : scope.get_countries()) {
      texts.push_back(to_text(country));
    }
    for(auto& venue : scope.get_venues()) {
      texts.push_back(to_text(venue));
    }
    for(auto& ticker : scope.get_tickers()) {
      texts.push_back(to_text(ticker));
    }
    if(n >= static_cast<int>(texts.size())) {
      return QString();
    }
    std::nth_element(texts.begin(), texts.begin() + n, texts.end());
    return texts[n];
  }

  template<>
  bool compare_text<Scope>(const AnyRef& left, const AnyRef& right) {
    auto& left_scope = any_cast<Scope>(left);
    auto& right_scope = any_cast<Scope>(right);
    auto i = 0;
    while(true) {
      auto left_text = find_smallest_text(left_scope, i);
      auto right_text = find_smallest_text(right_scope, i);
      auto comparator = left_text.compare(right_text);
      if(comparator == 0) {
        if(left_text.isEmpty()) {
          return false;
        }
        ++i;
      } else {
        return comparator < 0;
      }
    }
  }

  using EqualityTypes = ComparableTypes<bool, Quantity, gregorian::date, ptime,
    posix_time::time_duration, std::string, CountryCode, CurrencyId,
    DirectoryEntry, Money, Scope, OrderStatus, OrderType, PositionSide,
    PositionSideToken, Side, Ticker, TimeInForce, TradeSide, Venue, QColor,
    QKeySequence, QString, AccountListItem::Account, TimeAndSale::Condition,
    Task::State, optional<Money>, optional<Quantity>, optional<QString>,
    optional<std::string>>;

  template<typename T>
  std::size_t hash_item(const T& value) {
    if constexpr(requires { std::hash<T>()(value); }) {
      return std::hash<T>()(value);
    } else if constexpr(std::same_as<T, gregorian::date>) {
      return value.day_number();
    } else if constexpr(std::same_as<T, posix_time::time_duration>) {
      return std::hash<decltype(value.ticks())>()(value.ticks());
    } else if constexpr(std::same_as<T, ptime>) {
      auto hash = hash_item(value.date());
      hash_combine(hash, value.time_of_day().ticks());
      return hash;
    } else if constexpr(requires { value.m_side; }) {
      return hash_item(value.m_side);
    } else if constexpr(std::same_as<T, AccountListItem::Account>) {
      auto hash = hash_item(value.m_account);
      hash_combine(hash, qHash(value.m_name));
      return hash;
    } else if constexpr(std::same_as<T, TimeAndSale::Condition>) {
      auto hash = hash_item(value.m_type);
      hash_combine(hash, value.m_code);
      return hash;
    } else {
      return qHash(value);
    }
  }

  template<typename T>
  std::size_t hash_item(const optional<T>& value) {
    if(value) {
      return hash_item(*value);
    }
    return 0;
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

  auto to_text_helper(double value, const QLocale& locale) {
    auto text = locale.toString(value, 'f');
    text = text.remove(QRegExp("0+$"));
    text = text.remove(QRegExp("\\.$"));
    return text;
  }
}

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
  } else if(value.type() == typeid(DirectoryEntry)) {
    return QVariant::fromValue(std::any_cast<DirectoryEntry>(value));
  } else if(value.type() == typeid(Money)) {
    return QVariant::fromValue(std::any_cast<Money>(value));
  } else if(value.type() == typeid(OrderStatus)) {
    return QVariant::fromValue(std::any_cast<OrderStatus>(value));
  } else if(value.type() == typeid(OrderType)) {
    return QVariant::fromValue(std::any_cast<OrderType>(value));
  } else if(value.type() == typeid(PositionSide)) {
    return QVariant::fromValue(std::any_cast<PositionSide>(value));
  } else if(value.type() == typeid(PositionSideToken)) {
    return QVariant::fromValue(std::any_cast<PositionSideToken>(value));
  } else if(value.type() == typeid(Quantity)) {
    return QVariant::fromValue(std::any_cast<Quantity>(value));
  } else if(value.type() == typeid(Scope)) {
    return QVariant::fromValue(std::any_cast<Scope>(value));
  } else if(value.type() == typeid(Side)) {
    return QVariant::fromValue(std::any_cast<Side>(value));
  } else if(value.type() == typeid(Ticker)) {
    return QVariant::fromValue(std::any_cast<Ticker>(value));
  } else if(value.type() == typeid(TimeAndSale::Condition)) {
    return QVariant::fromValue(std::any_cast<TimeAndSale::Condition>(value));
  } else if(value.type() == typeid(TimeInForce)) {
    return QVariant::fromValue(std::any_cast<TimeInForce>(value));
  } else if(value.type() == typeid(TradeSide)) {
    return QVariant::fromValue(std::any_cast<TradeSide>(value));
  } else if(value.type() == typeid(Venue)) {
    return QVariant::fromValue(std::any_cast<Venue>(value));
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
  return to_text_helper(value, locale);
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
  auto local_time = to_local_time(time);
  auto current_time = to_local_time(posix_time::second_clock::universal_time());
  if(local_time.date() == current_time.date()) {
    return QString::fromStdString(to_simple_string(local_time).substr(12));
  }
  return QString::fromStdString(to_simple_string(local_time));
}

QString Spire::to_text(posix_time::time_duration time, const QLocale& locale) {
  return QString::fromStdString(to_simple_string(time));
}

QString Spire::to_text(const DirectoryEntry& entry, const QLocale& locale) {
  return QString::fromStdString(entry.m_name);
}

QString Spire::to_text(CountryCode code, const QLocale& locale) {
  auto& entry = COUNTRIES.from(code);
  return QString::fromStdString(entry.m_three_letter_code.get_data());
}

QString Spire::to_text(CurrencyId currency, const QLocale& locale) {
  auto& entry = CURRENCIES.from(currency);
  return QString::fromStdString(entry.m_code.get_data());
}

QString Spire::to_text(Money value, const QLocale& locale) {
  return QString::fromStdString(lexical_cast<std::string>(value));
}

QString Spire::to_text(Quantity value, const QLocale& locale) {
  return to_text_helper(static_cast<double>(value), locale);
}

QString Spire::to_text(
    const TimeAndSale::Condition& condition, const QLocale& locale) {
  return QString::fromStdString(condition.m_code);
}

const QString& Spire::to_text(
    TimeInForce time_in_force, const QLocale& locale) {
  auto type = time_in_force.get_type();
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

const QString& Spire::to_text(PositionSide side, const QLocale& locale) {
  if(side.m_side == Side::BID) {
    static const auto text = QObject::tr("Long");
    return text;
  } else if(side.m_side == Side::ASK) {
    static const auto text = QObject::tr("Short");
    return text;
  }
  static const auto text = QObject::tr("Flat");
  return text;
}

const QString& Spire::to_text(TradeSide side, const QLocale& locale) {
  if(side.m_side == Side::BID) {
    static const auto text = QObject::tr("Buy");
    return text;
  } else if(side.m_side == Side::ASK) {
    static const auto text = QObject::tr("Sell");
    return text;
  }
  static const auto text = QObject::tr("None");
  return text;
}

QString Spire::to_text(PositionSideToken token, const QLocale& locale) {
  return token.to_string();
}

QString Spire::to_text(const Scope& scope, const QLocale& locale) {
  if(scope.is_global()) {
    return QObject::tr("Global");
  }
  if(scope.get_tickers().size() == 1 && scope.get_venues().empty() &&
      scope.get_countries().empty()) {
    return to_text(*scope.get_tickers().begin(), locale);
  } else if(scope.get_venues().size() == 1 && scope.get_tickers().empty() &&
      scope.get_countries().empty()) {
    return to_text(Venue(*scope.get_venues().begin()), locale);
  } else if(scope.get_countries().size() == 1 &&
      scope.get_tickers().empty() && scope.get_venues().empty()) {
    return to_text(*scope.get_countries().begin(), locale);
  }
  return QString::fromStdString(scope.get_name());
}

const QString& Spire::to_text(Task::State state, const QLocale& locale) {
  if(state == Task::State::READY) {
    static const auto value = QObject::tr("Ready");
    return value;
  } else if(state == Task::State::INITIALIZING) {
    static const auto value = QObject::tr("Initializing");
    return value;
  } else if(state == Task::State::ACTIVE) {
    static const auto value = QObject::tr("Active");
    return value;
  } else if(state == Task::State::PENDING_CANCEL) {
    static const auto value = QObject::tr("Pending Cancel");
    return value;
  } else if(state == Task::State::CANCELED) {
    static const auto value = QObject::tr("Canceled");
    return value;
  } else if(state == Task::State::FAILED) {
    static const auto value = QObject::tr("Failed");
    return value;
  } else if(state == Task::State::COMPLETE) {
    static const auto value = QObject::tr("Complete");
    return value;
  } else {
    static const auto value = QObject::tr("");
    return value;
  }
}

QString Spire::to_text(const Ticker& ticker, const QLocale& locale) {
  return QString::fromStdString(to_string(ticker));
}

QString Spire::to_text(Venue venue, const QLocale& locale) {
  auto& entry = VENUES.from(venue);
  if(entry.m_venue) {
    return QString::fromStdString(entry.m_display_name);
  }
  return QString::fromStdString(venue.get_code().get_data());
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
  if(auto count = std::any_cast<DistinctCount>(&value)) {
    return to_text(*count);
  } else if(auto integer = std::any_cast<multiprecision::int128_t>(&value)) {
    return QString::fromStdString(integer->str());
  } else if(auto date = std::any_cast<gregorian::date>(&value)) {
    return to_text(*date, locale);
  } else if(auto timestamp = std::any_cast<ptime>(&value)) {
    return to_text(*timestamp, locale);
  } else if(auto duration = std::any_cast<posix_time::time_duration>(&value)) {
    return to_text(*duration, locale);
  } else if(auto account = std::any_cast<AccountListItem::Account>(&value)) {
    return to_text(*account, locale);
  } else if(auto country = std::any_cast<CountryCode>(&value)) {
    return to_text(*country, locale);
  } else if(auto currency = std::any_cast<CurrencyId>(&value)) {
    return to_text(*currency, locale);
  } else if(auto entry = std::any_cast<DirectoryEntry>(&value)) {
    return to_text(*entry, locale);
  } else if(auto money = std::any_cast<Money>(&value)) {
    return to_text(*money, locale);
  } else if(auto order_status = std::any_cast<OrderStatus>(&value)) {
    return to_text(*order_status, locale);
  } else if(auto order_type = std::any_cast<OrderType>(&value)) {
    return to_text(*order_type, locale);
  } else if(auto position_side = std::any_cast<PositionSide>(&value)) {
    return to_text(*position_side, locale);
  } else if(auto position_side_token =
      std::any_cast<PositionSideToken>(&value)) {
    return to_text(*position_side_token, locale);
  } else if(auto quantity = std::any_cast<Quantity>(&value)) {
    return to_text(*quantity, locale);
  } else if(auto scope = std::any_cast<Scope>(&value)) {
    return to_text(*scope, locale);
  } else if(auto side = std::any_cast<Side>(&value)) {
    return to_text(*side, locale);
  } else if(auto state = std::any_cast<Task::State>(&value)) {
    return to_text(*state, locale);
  } else if(auto ticker = std::any_cast<Ticker>(&value)) {
    return to_text(*ticker, locale);
  } else if(auto condition = std::any_cast<TimeAndSale::Condition>(&value)) {
    return to_text(*condition, locale);
  } else if(auto time_in_force = std::any_cast<TimeInForce>(&value)) {
    return to_text(*time_in_force, locale);
  } else if(auto trade_side = std::any_cast<TradeSide>(&value)) {
    return to_text(*trade_side, locale);
  } else if(auto venue = std::any_cast<Venue>(&value)) {
    return to_text(*venue, locale);
  } else if(auto key_sequence = std::any_cast<QKeySequence>(&value)) {
    return to_text(*key_sequence, locale);
  } else if(auto string = std::any_cast<std::string>(&value)) {
    return QString::fromStdString(*string);
  } else if(auto color = std::any_cast<QColor>(&value)) {
    return color->name();
  } else if(auto text = std::any_cast<QString>(&value)) {
    return *text;
  } else if(auto characters = std::any_cast<const char*>(&value)) {
    return QString::fromUtf8(*characters);
  } else if(auto boolean = std::any_cast<bool>(&value)) {
    return to_text(*boolean, locale);
  }
  auto text = QString();
  ComparableTypes<optional<Money>, optional<Quantity>>()(value,
    [&] (const auto& number) {
      text = format_number(number, locale);
      return true;
    });
  return text;
}

bool Spire::compare(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() != right.get_type()) {
    return false;
  }
  if(left.get_type() == typeid(TimeInForce)) {
    return compare_text<TimeInForce>(left, right);
  } else if(left.get_type() == typeid(Scope)) {
    return compare_text<Scope>(left, right);
  } else if(left.get_type() == typeid(DirectoryEntry)) {
    return DirectoryEntry::name_comparator(
      any_cast<DirectoryEntry>(left), any_cast<DirectoryEntry>(right));
  }
  return compare_any<bool, Quantity, optional<Quantity>, gregorian::date, ptime,
    posix_time::time_duration, std::string, CountryCode, CurrencyId, Money,
    optional<Money>, OrderStatus, OrderType, PositionSide, Side, Ticker,
    TradeSide, QKeySequence, QString>(left, right);
}

std::optional<std::size_t> Spire::hash_value(const AnyRef& value) {
  auto hash = std::optional<std::size_t>();
  EqualityTypes()(value, [&] (const auto& item) {
    hash = hash_item(item);
    return true;
  });
  return hash;
}

bool Spire::is_equal(const AnyRef& left, const AnyRef& right) {
  if(left.get_type() != right.get_type()) {
    return false;
  } else if(auto left_value = any_cast<DistinctCount>(&left)) {
    return *left_value == any_cast<DistinctCount>(right);
  } else if(auto left_integer = any_cast<multiprecision::int128_t>(&left)) {
    return *left_integer == any_cast<multiprecision::int128_t>(right);
  }
  return EqualityTypes()(left, right, [] (const auto& a, const auto& b) {
    return a == b;
  });
}

bool Spire::is_equal(const std::any& left, const std::any& right) {
  return is_equal(AnyRef(left), AnyRef(right));
}

bool Spire::is_equal(const std::any& left, const AnyRef& right) {
  return is_equal(AnyRef(left), right);
}

bool Spire::is_equal(const AnyRef& left, const std::any& right) {
  return is_equal(left, AnyRef(right));
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
  if(auto id = parse_currency(text.toStdString());
      id != CurrencyId::NONE) {
    return id;
  }
  return none;
}

template<>
optional<Money> Spire::from_text(const QString& text) {
  return try_parse_money(text.toStdString());
}

template<>
optional<Quantity> Spire::from_text(const QString& text) {
  return try_parse_quantity(text.toStdString());
}

template<>
optional<Scope> Spire::from_text(const QString& text) {
  return Scope(text.toStdString());
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
optional<Side> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Ask") || text == QObject::tr("Sell")) {
    return Side(Side::ASK);
  } else if(text == QObject::tr("Bid") || text == QObject::tr("Buy")) {
    return Side(Side::BID);
  } else if(text == QObject::tr("None")) {
    return Side(Side::NONE);
  }
  return none;
}

template<>
optional<PositionSide> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Long")) {
    return PositionSide(Side::BID);
  } else if(text == QObject::tr("Short")) {
    return PositionSide(Side::ASK);
  } else if(text == QObject::tr("Flat")) {
    return PositionSide(Side::NONE);
  }
  return none;
}

template<>
optional<TradeSide> Spire::from_text(const QString& text) {
  if(text == QObject::tr("Buy")) {
    return TradeSide(Side::BID);
  } else if(text == QObject::tr("Sell")) {
    return TradeSide(Side::ASK);
  } else if(text == QObject::tr("None")) {
    return TradeSide(Side::NONE);
  }
  return none;
}

template<>
optional<Ticker> Spire::from_text(const QString& text) {
  if(auto ticker = parse_ticker(text.toStdString())) {
    return ticker;
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
optional<Venue> Spire::from_text(const QString& text) {
  if(auto venue = parse_venue(text.toStdString())) {
    return venue;
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
