#ifndef NEXUS_TIMESTAMP_HPP
#define NEXUS_TIMESTAMP_HPP
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \class Timestamp
      \brief Used to represent timestamps in situations where multiple events
             can occur with the same time point but in a certain sequence.
      \tparam SequenceType The type used to order events with the same time
              point.
   */
  template<typename SequenceType = void>
  class Timestamp {
    public:

      //! The type used to order events with the same time point.
      using Sequence = SequenceType;

      //! Constructs an empty Timestamp.
      Timestamp();

      //! Copies a Timestamp.
      /*!
        \param timestamp The Timestamp to copy.
      */
      template<typename OtherSequence>
      Timestamp(const Timestamp<OtherSequence>& timestamp);

      //! Copies a Timestamp.
      /*!
        \param timestamp The Timestamp to copy.
      */
      Timestamp(const Timestamp<void>& timestamp);

      //! Constructs a Timestamp with a default Sequence.
      /*!
        \param timePoint The point in time to represent.
      */
      Timestamp(const boost::posix_time::ptime& timePoint);

      //! Constructs a Timestamp.
      /*!
        \param timePoint The point in time to represent.
        \param sequence The sequence number used to order two events that took
               place at the same point in time.
      */
      Timestamp(const boost::posix_time::ptime& timePoint,
        const Sequence& sequence);

      //! Constructs a Timestamp.
      /*!
        \param timePoint The point in time to represent.
      */
      Timestamp(boost::date_time::special_values timePoint);

      //! Returns the point in time represented by this Timestamp.
      const boost::posix_time::ptime& GetTimePoint() const;

      //! Returns the sequence.
      const Sequence& GetSequence() const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp comes before
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator <(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp comes before
      //! or is equal to another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator <=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is equal to
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator ==(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is not equal to
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator !=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is greater than or
      //! equal to another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator >=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is greater than
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator >(const Timestamp& timestamp) const;

    private:
      friend struct Beam::Serialization::Shuttle<Timestamp>;
      boost::posix_time::ptime m_timePoint;
      Sequence m_sequence;
  };

  /*! \class Timestamp<void>
      \brief A specialization of Timestamp used to represent a default sequence.
   */
  template<>
  class Timestamp<void> {
    public:

      //! Constructs an empty Timestamp.
      Timestamp();

      //! Constructs a Timestamp.
      /*!
        \param timePoint The point in time to represent.
      */
      Timestamp(const boost::posix_time::ptime& timePoint);

      //! Constructs a Timestamp.
      /*!
        \param timePoint The point in time to represent.
      */
      Timestamp(boost::date_time::special_values timePoint);

      //! Returns the point in time represented by this Timestamp.
      const boost::posix_time::ptime& GetTimePoint() const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp comes before
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator <(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp comes before
      //! or is equal to another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator <=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is equal to
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator ==(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is not equal to
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator !=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is greater than or
      //! equal to another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator >=(const Timestamp& timestamp) const;

      //! Returns <code>true</code> iff <i>this</i> Timestamp is greater than
      //! another.
      /*!
        \param timestamp The Timestamp to compare against.
      */
      bool operator >(const Timestamp& timestamp) const;

    private:
      friend struct Beam::Serialization::Shuttle<Timestamp>;
      boost::posix_time::ptime m_timePoint;
  };

  //! Makes a Timestamp.
  /*!
    \param timePoint The point in time to represent.
    \param sequence The sequence number used to order two events that took
           place at the same point in time.
  */
  template<typename Sequence>
  Timestamp<Sequence> MakeTimestamp(const boost::posix_time::ptime& timePoint,
      const Sequence& sequence) {
    return Timestamp<Sequence>(timePoint, sequence);
  }

  //! Makes a Timestamp.
  /*!
    \param timePoint The point in time to represent.
  */
  inline Timestamp<void> MakeTimestamp(
      const boost::posix_time::ptime& timePoint) {
    return Timestamp<void>(timePoint);
  }

  //! Increments a Timestamp by one sequence number.
  /*!
    \param timestamp The Timestamp to increment.
  */
  template<typename Sequence>
  Timestamp<Sequence> Increment(const Timestamp<Sequence>& timestamp) {
    Sequence s = timestamp.GetSequence();
    return Timestamp<Sequence>(timestamp.GetTimePoint(), ++s);
  }

  template<typename SequenceType>
  Timestamp<SequenceType>::Timestamp()
      : m_timePoint(boost::posix_time::not_a_date_time),
        m_sequence() {}

  template<typename SequenceType>
  template<typename OtherSequence>
  Timestamp<SequenceType>::Timestamp(const Timestamp<OtherSequence>& timestamp)
      : m_timePoint(timestamp.GetTimePoint()),
        m_sequence(timestamp.GetSequence()) {}

  template<typename SequenceType>
  Timestamp<SequenceType>::Timestamp(const Timestamp<void>& timestamp)
      : m_timePoint(timestamp.GetTimePoint()),
        m_sequence() {}

  template<typename SequenceType>
  Timestamp<SequenceType>::Timestamp(const boost::posix_time::ptime& timePoint)
      : m_timePoint(timePoint),
        m_sequence() {}

  template<typename SequenceType>
  Timestamp<SequenceType>::Timestamp(const boost::posix_time::ptime& timePoint,
      const Sequence& sequence)
      : m_timePoint(timePoint),
        m_sequence(sequence) {}

  template<typename SequenceType>
  Timestamp<SequenceType>::Timestamp(boost::date_time::special_values timePoint)
      : m_timePoint(timePoint),
        m_sequence() {}

  template<typename SequenceType>
  const boost::posix_time::ptime& Timestamp<SequenceType>::
      GetTimePoint() const {
    return m_timePoint;
  }

  template<typename SequenceType>
  const typename Timestamp<SequenceType>::Sequence& Timestamp<SequenceType>::
      GetSequence() const {
    return m_sequence;
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator <(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) <
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator <=(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) <=
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator ==(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) ==
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator !=(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) !=
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator >=(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) >=
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  template<typename SequenceType>
  bool Timestamp<SequenceType>::operator >(const Timestamp& timestamp) const {
    return std::tie(m_timePoint, m_sequence) >
      std::tie(timestamp.m_timePoint, timestamp.m_sequence);
  }

  inline Timestamp<void>::Timestamp()
      : m_timePoint(boost::posix_time::not_a_date_time) {}

  inline Timestamp<void>::Timestamp(const boost::posix_time::ptime& timePoint)
      : m_timePoint(timePoint) {}

  inline Timestamp<void>::Timestamp(boost::date_time::special_values timePoint)
      : m_timePoint(timePoint) {}

  inline const boost::posix_time::ptime& Timestamp<void>::GetTimePoint() const {
    return m_timePoint;
  }

  inline bool Timestamp<void>::operator <(const Timestamp& timestamp) const {
    return m_timePoint < timestamp.m_timePoint;
  }

  inline bool Timestamp<void>::operator <=(const Timestamp& timestamp) const {
    return m_timePoint <= timestamp.m_timePoint;
  }

  inline bool Timestamp<void>::operator ==(const Timestamp& timestamp) const {
    return m_timePoint == timestamp.m_timePoint;
  }

  inline bool Timestamp<void>::operator !=(const Timestamp& timestamp) const {
    return m_timePoint != timestamp.m_timePoint;
  }

  inline bool Timestamp<void>::operator >=(const Timestamp& timestamp) const {
    return m_timePoint >= timestamp.m_timePoint;
  }

  inline bool Timestamp<void>::operator >(const Timestamp& timestamp) const {
    return m_timePoint > timestamp.m_timePoint;
  }
}

namespace Beam {
namespace Serialization {
  template<typename SequenceType>
  struct Shuttle<Nexus::Timestamp<SequenceType>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Timestamp<SequenceType>& value,
        unsigned int version) const {
      shuttle.Shuttle("time_point", value.m_timePoint);
      shuttle.Shuttle("sequence", value.m_sequence);
    }
  };

  template<>
  struct Shuttle<Nexus::Timestamp<void>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Nexus::Timestamp<void>& value,
        unsigned int version) const {
      shuttle.Shuttle("time_point", value.m_timePoint);
    }
  };
}
}

#endif
