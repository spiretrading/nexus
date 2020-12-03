#ifndef SPIRE_TIMERANGEPARAMETERNODE_HPP
#define SPIRE_TIMERANGEPARAMETERNODE_HPP
#include <Beam/Queries/Range.hpp>
#include <Beam/TimeService/TimeService.hpp>
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Common/CanvasNode.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /*! \class TimeRangeParameterNode
      \brief A CanvasNode that allows the user to specify a TimeRangeParameter.
   */
  class TimeRangeParameterNode : public CanvasNode {
    public:

      //! Constructs a TimeRangeParameterNode.
      TimeRangeParameterNode();

      //! Constructs a TimeRangeParameterNode.
      /*!
        \param startTime The range's start time.
        \param endTime The range's end time.
      */
      TimeRangeParameterNode(const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime);

      //! Returns the range's start time.
      const TimeRangeParameter& GetStartTime() const;

      //! Returns the range's end time.
      const TimeRangeParameter& GetEndTime() const;

      //! Sets the time range.
      /*!
        \param startTime The range's start time.
        \param endTime The range's end time.
      */
      std::unique_ptr<TimeRangeParameterNode> SetTimeRange(
        const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime) const;

      //! Returns the time range as a query.
      /*!
        \param timeClient The TimeClient used to resolve the query.
        \return The time range as a query.
      */
      Beam::Queries::Range GetTimeRangeQuery(
        Beam::TimeService::TimeClientBox& timeClient) const;

      virtual void Apply(CanvasNodeVisitor& visitor) const;

    protected:
      virtual std::unique_ptr<CanvasNode> Clone() const;

      virtual std::unique_ptr<CanvasNode> Reset() const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      TimeRangeParameter m_startTime;
      TimeRangeParameter m_endTime;

      TimeRangeParameterNode(Beam::Serialization::ReceiveBuilder);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void TimeRangeParameterNode::Shuttle(Shuttler& shuttle,
      unsigned int version) {
    CanvasNode::Shuttle(shuttle, version);
    shuttle.Shuttle("start_time", m_startTime);
    shuttle.Shuttle("end_time", m_endTime);
  }
}

#endif
