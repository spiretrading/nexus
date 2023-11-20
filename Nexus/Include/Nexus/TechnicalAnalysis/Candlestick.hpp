#ifndef NEXUS_CANDLESTICK_HPP
#define NEXUS_CANDLESTICK_HPP
#include <algorithm>
#include <tuple>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/TechnicalAnalysis/TechnicalAnalysis.hpp"

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
      Candlestick();

      /**
       * Constructs a Candlestick with a specified domain.
       * @param start The start of the domain represented.
       * @param end The start of the domain represented.
       */
      Candlestick(Domain start, Domain end);

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
        Range low);

      /** Returns the start of the domain represented. */
      Domain GetStart() const;

      /** Sets the domain's start range. */
      void SetStart(Domain start);

      /** Returns the end of the domain represented. */
      Domain GetEnd() const;

      /** Sets the domain's end range. */
      void SetEnd(Domain start);

      /** Returns the Candlestick's opening value. */
      Range GetOpen() const;

      /** Returns the Candlestick's closing value. */
      Range GetClose() const;

      /** Returns the Candlestick's highest value. */
      Range GetHigh() const;

      /** Returns the Candlestick's lowest value. */
      Range GetLow() const;

      /**
       * Updates the Candlestick with a value.
       * @param value The value to update this Candlestick with.
       */
      void Update(Range value);

      bool operator ==(const Candlestick& candlestick) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<Candlestick>;
      bool m_hasOpen;
      Domain m_start;
      Domain m_end;
      Range m_open;
      Range m_close;
      Range m_high;
      Range m_low;
  };

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick()
    : m_hasOpen(false),
      m_start(),
      m_end(),
      m_open(),
      m_close(),
      m_high(),
      m_low() {}

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick(Domain start, Domain end)
    : m_hasOpen(false),
      m_start(std::move(start)),
      m_end(std::move(end)) {}

  template<typename D, typename R>
  Candlestick<D, R>::Candlestick(Domain start, Domain end, Range open,
    Range close, Range high, Range low)
    : m_hasOpen(true),
      m_start(std::move(start)),
      m_end(std::move(end)),
      m_open(std::move(open)),
      m_close(std::move(close)),
      m_high(std::move(high)),
      m_low(std::move(low)) {}

  template<typename D, typename R>
  typename Candlestick<D, R>::Domain Candlestick<D, R>::GetStart() const {
    return m_start;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::SetStart(Domain start) {
    m_start = start;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Domain Candlestick<D, R>::GetEnd() const {
    return m_end;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::SetEnd(Domain end) {
    m_end = end;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::GetOpen() const {
    return m_open;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::GetClose() const {
    return m_close;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::GetHigh() const {
    return m_high;
  }

  template<typename D, typename R>
  typename Candlestick<D, R>::Range Candlestick<D, R>::GetLow() const {
    return m_low;
  }

  template<typename D, typename R>
  void Candlestick<D, R>::Update(Range value) {
    if(m_hasOpen) {
      m_close = value;
      m_high = std::max(m_high, value);
      m_low = std::min(m_low, value);
      return;
    }
    m_open = value;
    m_close = value;
    m_high = value;
    m_low = value;
    m_hasOpen = true;
  }
}

namespace Beam::Serialization {
  template<typename D, typename R>
  struct Shuttle<Nexus::TechnicalAnalysis::Candlestick<D, R>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::TechnicalAnalysis::Candlestick<D, R>& value,
        unsigned int version) {
      shuttle.Shuttle("has_open", value.m_hasOpen);
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
