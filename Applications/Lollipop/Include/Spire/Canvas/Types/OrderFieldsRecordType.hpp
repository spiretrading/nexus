#ifndef SPIRE_ORDERFIELDSRECORDTYPE_HPP
#define SPIRE_ORDERFIELDSRECORDTYPE_HPP
#include <memory>
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing an OrderFields instance.
  const RecordType& GetOrderFieldsRecordType();

  /*! \struct OrderFieldsToRecordConverter
      \brief Defines a conversion from an OrderFields instance to a Record.
   */
  struct OrderFieldsToRecordConverter {

    //! Converts an OrderFields instance into a Record.
    /*!
      \param value The OrderFields instance to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(const Nexus::OrderFields& value) const;
  };
}

#endif
