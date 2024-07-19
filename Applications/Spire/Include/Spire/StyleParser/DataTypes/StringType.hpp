#ifndef SPIRE_STRING_TYPE_HPP
#define SPIRE_STRING_TYPE_HPP
#include "Spire/StyleParser/DataTypes/DataType.hpp"

namespace Spire {

  /** Represents the string type. */
  class StringType final : public DataType {
    public:

      /** Constructs a StringType. */
      StringType();

    protected:
      bool is_equal(const DataType& rhs) const override;
  };
}

#endif
