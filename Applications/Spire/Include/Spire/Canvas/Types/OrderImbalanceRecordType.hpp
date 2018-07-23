#ifndef SPIRE_ORDERIMBALANCERECORDTYPE_HPP
#define SPIRE_ORDERIMBALANCERECORDTYPE_HPP
#include "Nexus/MarketDataService/MarketWideDataQuery.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing an OrderImbalance.
  const RecordType& GetOrderImbalanceRecordType();

  /*! \struct OrderImbalanceToRecordConverter
      \brief Defines a conversion from an OrderImbalance to a Record.
   */
  struct OrderImbalanceToRecordConverter {

    //! Converts an OrderImbalance into a Record.
    /*!
      \param value The OrderImbalance to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(const Nexus::SequencedOrderImbalance& value) const;
  };
}

#endif
