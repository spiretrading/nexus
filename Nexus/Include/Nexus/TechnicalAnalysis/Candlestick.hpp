#ifndef NEXUS_CANDLESTICK_HPP
#define NEXUS_CANDLESTICK_HPP
#include <algorithm>
#include <tuple>
#include <Beam/Serialization/DataShuttle.hpp>
#include "Nexus/TechnicalAnalysis/TechnicalAnalysis.hpp"

namespace Nexus {
namespace TechnicalAnalysis {

  /*! \class Candlestick
      \brief Represents a candlestick displayed on a chart.
      \tparam DomainType The type used to represent the domain (eg. time).
      \tparam RangeType The type used to represent the range (eg. price).
   */
  template<typename DomainType, typename RangeType>
  class Candlestick {
    public:

      //! The type used to represent the domain (eg. time).
      typedef DomainType Domain;

      //! The type used to represent the range (eg. price).
      typedef RangeType Range;

      //! Constructs an empty Candlestick.
      Candlestick();

      //! Constructs a Candlestick with a specified domain.
      /*!
        \param start The start of the domain represented.
        \param end The start of the domain represented.
      */
      Candlestick(Domain start, Domain end);

      //! Constructs a Candlestick with an initial set of values.
      /*!
        \param start The start of the domain represented.
        \param end The start of the domain represented.
        \param open The first value represented.
        \param close The last value represented.
        \param high The highest value represented.
        \param low The lowest value represented.
      */
      Candlestick(Domain start, Domain end, Range open, Range close,
        Range high, Range low);

      //! Returns <code>true</code> iff this Candlestick is equal to another.
      /*!
        \param candlestick The Candlestick to compare to.
        \return <code>true</code> iff the two values are equal.
      */
      bool operator ==(const Candlestick& candlestick) const;

      //! Returns <code>true</code> iff this Candlestick is not equal to
      //! another.
      /*!
        \param candlestick The Candlestick to compare to.
        \return <code>true</code> iff the two values are not equal.
      */
      bool operator !=(const Candlestick& candlestick) const;

      //! Returns the start of the domain represented.
      Domain GetStart() const;

      //! Sets the domain's start range.
      void SetStart(Domain start);

      //! Returns the end of the domain represented.
      Domain GetEnd() const;

      //! Sets the domain's end range.
      void SetEnd(Domain start);

      //! Returns the Candlestick's opening value.
      Range GetOpen() const;

      //! Returns the Candlestick's closing value.
      Range GetClose() const;

      //! Returns the Candlestick's highest value.
      Range GetHigh() const;

      //! Returns the Candlestick's lowest value.
      Range GetLow() const;

      //! Updates the Candlestick with a value.
      /*!
        \param value The value to update this Candlestick with.
      */
      void Update(Range value);

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

  template<typename DomainType, typename RangeType>
  Candlestick<DomainType, RangeType>::Candlestick()
      : m_hasOpen(false),
        m_start(),
        m_end(),
        m_open(),
        m_close(),
        m_high(),
        m_low() {}

  template<typename DomainType, typename RangeType>
  Candlestick<DomainType, RangeType>::Candlestick(Domain start, Domain end)
      : m_hasOpen(false),
        m_start(start),
        m_end(end) {}

  template<typename DomainType, typename RangeType>
  Candlestick<DomainType, RangeType>::Candlestick(Domain start, Domain end,
      Range open, Range close, Range high, Range low)
      : m_hasOpen(true),
        m_start(start),
        m_end(end),
        m_open(open),
        m_close(close),
        m_high(high),
        m_low(low) {}

  template<typename DomainType, typename RangeType>
  bool Candlestick<DomainType, RangeType>::operator ==(
      const Candlestick& candlestick) const {
    return std::tie(m_hasOpen, m_start, m_end, m_open, m_close, m_high,
      m_low) == std::tie(candlestick.m_hasOpen, candlestick.m_start,
      candlestick.m_end, candlestick.m_open, candlestick.m_close,
      candlestick.m_high, candlestick.m_low);
  }

  template<typename DomainType, typename RangeType>
  bool Candlestick<DomainType, RangeType>::operator !=(
      const Candlestick& candlestick) const {
    return !(*this == candlestick);
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Domain
      Candlestick<DomainType, RangeType>::GetStart() const {
    return m_start;
  }

  template<typename DomainType, typename RangeType>
  void Candlestick<DomainType, RangeType>::SetStart(Domain start) {
    m_start = start;
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Domain
      Candlestick<DomainType, RangeType>::GetEnd() const {
    return m_end;
  }

  template<typename DomainType, typename RangeType>
  void Candlestick<DomainType, RangeType>::SetEnd(Domain end) {
    m_end = end;
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Range
      Candlestick<DomainType, RangeType>::GetOpen() const {
    return m_open;
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Range
      Candlestick<DomainType, RangeType>::GetClose() const {
    return m_close;
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Range
      Candlestick<DomainType, RangeType>::GetHigh() const {
    return m_high;
  }

  template<typename DomainType, typename RangeType>
  typename Candlestick<DomainType, RangeType>::Range
      Candlestick<DomainType, RangeType>::GetLow() const {
    return m_low;
  }

  template<typename DomainType, typename RangeType>
  void Candlestick<DomainType, RangeType>::Update(Range value) {
    if(!m_hasOpen) {
      m_open = value;
      m_close = value;
      m_high = value;
      m_low = value;
      m_hasOpen = true;
      return;
    }
    m_close = value;
    m_high = std::max(m_high, value);
    m_low = std::min(m_low, value);
  }
}
}

namespace Beam {
namespace Serialization {
  template<typename DomainType, typename RangeType>
  struct Shuttle<Nexus::TechnicalAnalysis::Candlestick<DomainType, RangeType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::TechnicalAnalysis::Candlestick<DomainType, RangeType>& value,
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
}

#endif
