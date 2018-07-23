#ifndef SPIRE_QUOTERECORDTYPE_HPP
#define SPIRE_QUOTERECORDTYPE_HPP
#include <memory>
#include "Nexus/Definitions/Definitions.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing a Quote.
  const RecordType& GetQuoteRecordType();

  /*! \struct QuoteToRecordConverter
      \brief Defines a conversion from a Quote to a Record.
   */
  struct QuoteToRecordConverter {

    //! Converts a Quote into a Record.
    /*!
      \param value The Quote to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(const Nexus::Quote& value) const;
  };
}

#endif
