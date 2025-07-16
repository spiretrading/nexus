#ifndef NEXUS_CANDLESTICK_HPP
#define NEXUS_CANDLESTICK_HPP
#include <algorithm>
#include <ostream>
#include <type_traits>
#include <utility>
#include <Beam/Serialization/DataShuttle.hpp>

namespace Nexus::TechnicalAnalysis {

  /**
   * Represents a candlestick displayed on a chart.
   * @param <D> The type used to represent the domain (eg. time).
   * @param <R> The type used to represent the range (eg. price).
   */
  template<typename D, typename R>
  class Candlestick {
    public:

      /** The type used to represent the domain (eg. time). */
      using Domain = D;

      /** The type used to represent the range (eg. price). */
      using Range = R;

      /** Constructs an empty Candlestick. */
      Candlestick() noexcept(std::is_nothrow_default_constructible_v<Domain> &&
        std::is_nothrow_default_constructible_v<Range>);

      /**
       * Constructs a Candlestick with a specified domain.
       * @param start The start of the domain represented.
       * @param end The start of the domain represented.
       */
      Candlestick(Domain start, Domain end) noexcept(
        std::is_nothrow_move_constructible_v<Domain> &&
        std::is_nothrow_default_constructible_v<Range>);

      /**
       * Constructs a Candlestick with an initial set of values.
       * @param start The start of the domain represented.
       * @param end The start of the domain represented.
       * @param open The first value represented.
       * @param close The last value represented.
       * @param high The highest value represented.
       * @param low The lowest value represented.
       */
      Candlestick(Domain start, Domain end, Range open, Range close, Range high,
        Range low) noexcept(std::is_nothrow_move_constructible_v<Domain> &&
        std::is_nothrow_move_constructible_v<Range>);

      /** Returns the start of the domain represented. */
      Domain get_start() const;

      /** Sets the domain's start range. */
      void set_start(Domain start);

      /** Returns the end of the domain represented. */
      Domain get_end() const;

      /** Sets the domain's end range. */
      void set_end(Domain end);

      /** Returns the Candlestick's opening value. */
      Range get_open() const;

      /** Returns the Candlestick's closing value. */
      Range get_close() const;

      /** Returns the Candlestick's highest value. */
      Range get_high() const;

      /** Returns the Candlestick's lowest value. */
      Range get_low() const;

      /**
       * Updates the Candlestick with a value.
       * @param value The value to update this Candlestick with.
       */
      void update(Range value);

      bool operator ==(const Candlestick&) const = default;

    private:
      template<typename U, typename V>
      friend std::ostream& operator <<(std::ostream&, const Candlestick<U, V>&);
      friend struct Beam::Serialization::Shuttle<Candlestick>;
      bool m_has_open;
      Domain m_start;
      Domain m_end;
      Range m_open;
      Range m_close;
      Range m_high;
      Range m_low;
  };

  template<typename D, typename R>
  std::ostream& operator <<(
      std::ostream& out, const Candlestick<D, R>& candlestick) {
    if(!candlestick.m_has_open) {
      return out << '(' << candlestick.get_start() << ", " <<
        candlestick.get_end() << ')';
    }
    return out << '(' << candlestick.get_start() << ", " <<
      candlestick.get_end() << ", " << candlestick.get_open() << ", " <<
      candlestick.get_high() << ", " << candlestick.get_low() << ", " <<
      candlestick.get_close() << ")";
  }

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick() noexcept(
    std::is_nothrow_default_constructible_v<Domain> &&
    std::is_nothrow_default_constructible_v<Range>)
    : m_has_open(false),
      m_start(),
      m_end(),
      m_open(),
      m_close(),
      m_high(),
      m_low() {}

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick(Domain start, Domain end) noexcept(
    std::is_nothrow_move_constructible_v<Domain> &&
    std::is_nothrow_default_constructible_v<Range>)
    : m_has_open(false),
      m_start(std::move(start)),
      m_end(std::move(end)) {}

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick(Domain start, Domain end, Range open,
    Range close, Range high, Range low) noexcept(
    std::is_nothrow_move_constructible_v<Domain> &&
    std::is_nothrow_move_constructible_v<Range>)
    : m_has_open(true),
      m_start(std::move(start)),
      m_end(std::move(end)),
      m_open(std::move(open)),
      m_close(std::move(close)),
      m_high(std::move(high)),
      m_low(std::move(low)) {}

  template<typename D, typename R>
  typename Candlestick<D, R>::Domain Candlestick<D, R>::get_start() const {
    return m_start;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::set_start(Domain start) {
    m_start = std::move(start);
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Domain Candlestick<D, R>::get_end() const {
    return m_end;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::set_end(Domain end) {
    m_end = std::move(end);
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::get_open() const {
    return m_open;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::get_close() const {
    return m_close;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::get_high() const {
    return m_high;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::get_low() const {
    return m_low;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::update(Range value) {
    if(m_has_open) {
      m_close = value;
      m_high = std::max(m_high, value);
      m_low = std::min(m_low, value);
      return;
    }
    m_open = value;
    m_close = value;
    m_high = value;
    m_low = value;
    m_has_open = true;
  }
}

namespace Beam::Serialization {
  template<typename D, typename R>
  struct Shuttle<Nexus::TechnicalAnalysis::Candlestick<D, R>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::TechnicalAnalysis::Candlestick<D, R>& value,
        unsigned int version) {
      shuttle.Shuttle("has_open", value.m_has_open);
      shuttle.Shuttle("start", value.m_start);
      shuttle.Shuttle("end", value.m_end);
      shuttle.Shuttle("open", value.m_open);
      shuttle.Shuttle("close", value.m_close);
      shuttle.Shuttle("high", value.m_high);
      shuttle.Shuttle("low", value.m_low);
    }
  };
}

#endif
