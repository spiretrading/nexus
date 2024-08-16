#ifndef SPIRE_TIMEANDSALERECORDTYPE_HPP
#define SPIRE_TIMEANDSALERECORDTYPE_HPP
#include "Nexus/MarketDataService/SecurityMarketDataQuery.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing a TimeAndSale.
  const RecordType& GetTimeAndSaleRecordType();

  /*! \struct TimeAndSaleToRecordConverter
      \brief Defines a conversion from a TimeAndSale to a Record.
   */
  struct TimeAndSaleToRecordConverter {

    //! Converts a value into a Record.
    /*!
      \param value The value to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(const Nexus::SequencedTimeAndSale& value) const;
  };
}

#endif
