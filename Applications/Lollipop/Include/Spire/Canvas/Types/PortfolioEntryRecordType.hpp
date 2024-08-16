#ifndef SPIRE_PORTFOLIOENTRYRECORDTYPE_HPP
#define SPIRE_PORTFOLIOENTRYRECORDTYPE_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Returns an instance of a RecordType representing a portfolio's position.
  const RecordType& GetPortfolioEntryRecordType();

  /*! \struct PortfolioEntryToRecordConverter
      \brief Defines a conversion from a Portfolio's UpdateEntry to a Record.
   */
  struct PortfolioEntryToRecordConverter {

    //! Converts a Portfolio's UpdateEntry to a Record.
    /*!
      \param value The Portfolio UpdateEntry to convert.
      \return The Record representation of the <i>value</i>.
    */
    Record operator ()(const SpirePortfolio::UpdateEntry& value) const;
  };
}

#endif
