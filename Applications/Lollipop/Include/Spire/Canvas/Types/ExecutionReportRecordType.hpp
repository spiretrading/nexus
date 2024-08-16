#ifndef SPIRE_EXECUTIONREPORTRECORDTYPE_HPP
#define SPIRE_EXECUTIONREPORTRECORDTYPE_HPP
#include <memory>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing an ExecutionReport.
  const RecordType& GetExecutionReportRecordType();

  /*! \struct ExecutionReportToRecordConverter
      \brief Defines a conversion from an ExecutionReport to a Record.
   */
  struct ExecutionReportToRecordConverter {

    //! Converts an ExecutionReport into a Record.
    /*!
      \param value The ExecutionReport to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(
      const Nexus::OrderExecutionService::ExecutionReport& value) const;
  };
}

#endif
