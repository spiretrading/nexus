#include "Spire/Ui/TableAggregation.hpp"
#include <cmath>
#include <limits>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <QHash>
#include "Spire/Ui/AccountListItem.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/Details/DefaultTableAggregation.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::multiprecision;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Details;

namespace {
  template<typename... T>
  bool visit_as(const AnyRef& value, const auto& visit) {
    auto match = [&]<typename N> {
      if(auto item = any_cast<N>(&value)) {
        visit(*item);
      } else if(auto item = any_cast<optional<N>>(&value)) {
        if(*item) {
          visit(**item);
        }
      } else {
        return false;
      }
      return true;
    };
    return (match.template operator()<T>() || ...);
  }

  void visit_value(const AnyRef& value, const auto& visit) {
    if(!value.has_value()) {
      return;
    }
    if(visit_as<int, unsigned int, long, unsigned long, long long,
        unsigned long long, short, unsigned short, signed char, unsigned char,
        double, float, long double, Quantity, Money>(value, visit)) {
      return;
    }
    auto visit_text = [&] (const auto& text) {
      visit(AnyRef(text));
    };
    if(!visit_as<QString, std::string, Side, PositionSide, TradeSide,
        CurrencyId, Ticker, Venue, CountryCode, OrderStatus, OrderType,
        TimeInForce, TimeAndSale::Condition, Scope, DirectoryEntry,
        AccountListItem::Account, Task::State, PositionSideToken, date, ptime,
        time_duration, QColor, QKeySequence, bool>(value, visit_text)) {
      visit(value);
    }
  }

  template<typename T>
  QString frequency_key(const T& value) {
    if constexpr(std::signed_integral<T>) {
      return to_text(static_cast<std::int64_t>(value));
    } else if constexpr(std::unsigned_integral<T>) {
      return to_text(static_cast<std::uint64_t>(value));
    } else if constexpr(std::floating_point<T>) {
      return to_text(static_cast<double>(value));
    } else {
      return to_text(value);
    }
  }

  QString frequency_key(const AnyRef& value) {
    if(auto timestamp = any_cast<ptime>(&value)) {
      return QString::fromStdString(to_iso_extended_string(*timestamp));
    }
    return to_text(value);
  }

  struct RealSum {
    cpp_int m_significand = 0;
    int m_exponent = 0;
    int m_nan = 0;
    int m_positive_infinity = 0;
    int m_negative_infinity = 0;

    void apply(long double value, int count) {
      if(std::isnan(value)) {
        m_nan += count;
      } else if(std::isinf(value)) {
        if(value > 0) {
          m_positive_infinity += count;
        } else {
          m_negative_infinity += count;
        }
      } else if(value != 0) {
        constexpr auto DIGITS = std::numeric_limits<long double>::digits;
        auto exponent = 0;
        auto fraction = std::frexp(value * count, &exponent);
        auto significand = cpp_int(std::ldexp(fraction, DIGITS));
        exponent -= DIGITS;
        if(m_significand == 0) {
          m_exponent = exponent;
        } else if(exponent < m_exponent) {
          m_significand <<= m_exponent - exponent;
          m_exponent = exponent;
        } else {
          significand <<= exponent - m_exponent;
        }
        m_significand += significand;
      }
    }

    cpp_bin_float_50 get() const {
      if(m_nan != 0 || (m_positive_infinity != 0 && m_negative_infinity != 0)) {
        return std::numeric_limits<long double>::quiet_NaN();
      } else if(m_positive_infinity != 0) {
        return std::numeric_limits<long double>::infinity();
      } else if(m_negative_infinity != 0) {
        return -std::numeric_limits<long double>::infinity();
      }
      return ldexp(cpp_bin_float_50(m_significand), m_exponent);
    }
  };

  class DefaultAggregation final : public TableAggregation {
    public:
      DefaultAggregation()
        : m_integer(0),
          m_integer_count(0),
          m_real_count(0),
          m_quantity_count(0),
          m_money_count(0),
          m_text_count(0),
          m_numeric_count(0) {}

      void set_visitor(std::function<void (const TableValueVisitor&)> visit) {
        if(m_numeric_count == 0 && m_text_count == 0) {
          m_visit = std::move(visit);
        }
      }

      void add(const AnyRef& value) override {
        apply(value, 1);
      }

      void remove(const AnyRef& value) override {
        apply(value, -1);
      }

      void update(const AnyRef& previous, const AnyRef& current) override {
        remove(previous);
        add(current);
      }

      std::any get() const override {
        if(m_text_count != 0) {
          if(m_numeric_count != 0 && m_visit) {
            auto frequencies = QHash<QString, int>();
            m_visit([&] (const AnyRef& value) {
              visit_value(value, [&] (const auto& contribution) {
                ++frequencies[frequency_key(contribution)];
              });
            });
            m_frequencies = std::move(frequencies);
            m_visit = {};
          }
          return QStringLiteral("(%1)").arg(m_frequencies.size());
        } else if(m_money_count != 0 || m_quantity_count != 0) {
          auto scalar = m_real.get() + cpp_bin_float_50(m_integer);
          auto sum = Quantity::from_representation(static_cast<double>(
            m_quantity.get() + scalar * Quantity::MULTIPLIER));
          if(m_money_count != 0) {
            return Money(sum);
          }
          return sum;
        } else if(m_real_count != 0) {
          return static_cast<double>(
            m_real.get() + cpp_bin_float_50(m_integer));
        } else if(m_integer_count == 0) {
          return {};
        } else if(m_integer >= std::numeric_limits<std::int64_t>::min() &&
            m_integer <= std::numeric_limits<std::int64_t>::max()) {
          return m_integer.convert_to<std::int64_t>();
        } else if(m_integer >= 0 &&
            m_integer <= std::numeric_limits<std::uint64_t>::max()) {
          return m_integer.convert_to<std::uint64_t>();
        }
        return m_integer.str();
      }

    private:
      int128_t m_integer;
      RealSum m_real;
      RealSum m_quantity;
      int m_integer_count;
      int m_real_count;
      int m_quantity_count;
      int m_money_count;
      int m_text_count;
      int m_numeric_count;
      mutable QHash<QString, int> m_frequencies;
      mutable std::function<void (const TableValueVisitor&)> m_visit;

      void apply(const AnyRef& value, int count) {
        visit_value(value, [&]<typename T> (const T& contribution) {
          if constexpr(std::same_as<T, AnyRef>) {
            m_text_count += count;
          } else {
            if constexpr(std::integral<T>) {
              m_integer_count += count;
              m_integer += int128_t(contribution) * count;
            } else if constexpr(std::floating_point<T>) {
              m_real_count += count;
              m_real.apply(contribution, count);
              if(m_real_count == 0) {
                m_real = RealSum();
              }
            } else {
              if constexpr(std::same_as<T, Money>) {
                m_money_count += count;
              } else {
                m_quantity_count += count;
              }
              auto quantity = static_cast<Quantity>(contribution);
              m_quantity.apply(quantity.get_representation(), count);
              if(m_quantity_count == 0 && m_money_count == 0) {
                m_quantity = RealSum();
              }
            }
            m_numeric_count += count;
            if(m_visit) {
              return;
            }
          }
          auto key = frequency_key(contribution);
          if(count > 0) {
            ++m_frequencies[key];
            return;
          }
          auto i = m_frequencies.find(key);
          if(i != m_frequencies.end()) {
            --i.value();
            if(i.value() == 0) {
              m_frequencies.erase(i);
            }
          }
        });
      }
  };
}

std::unique_ptr<TableAggregation> Spire::make_default_table_aggregation() {
  return std::make_unique<DefaultAggregation>();
}

void Spire::Details::set_table_value_visitor(TableAggregation& aggregation,
    std::function<void (const TableValueVisitor&)> visit) {
  static_cast<DefaultAggregation&>(aggregation).set_visitor(std::move(visit));
}

bool Spire::Details::is_default_table_aggregation(
    const TableAggregation* aggregation) {
  return dynamic_cast<const DefaultAggregation*>(aggregation);
}
