#ifndef SPIRE_TIME_AND_SALES_MODEL_HPP
#define SPIRE_TIME_AND_SALES_MODEL_HPP
#include "spire/time_and_sales/time_and_sales.hpp"

namespace spire {

  //! Models the time and sales for a single security.
  class time_and_sales_model {
    public:

      //! The position of a time and sale print relative to the BBO price.
      enum class price_range {

        //! The BBO price was not known.
        UNKNOWN,

        //! The time and sale price was greater than the BBO ask.
        ABOVE_ASK,

        //! The time and sale price was equal to the BBO ask.
        AT_ASK,

        //! The time and sale price is inbetween the BBO.
        INSIDE,

        //! The time and sale price is equal to the BBO bid.
        AT_BID,

        //! The time and sale price is less than the BBO bid.
        BELOW_BID,
      };

      //! The available columns to display.
      enum class columns {

        //! The time column.
        TIME_COLUMN,

        //! The price column.
        PRICE_COLUMN,

        //! The size column.
        SIZE_COLUMN,

        //! The market column.
        MARKET_COLUMN,

        //! The sales condition column.
        CONDITION_COLUMN,
      };

      //! The number of columns in this model.
      static const auto COLUMN_COUNT = 5;
  };
}

#endif
