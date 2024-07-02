#ifndef SPIRE_INTEGER_TYPE_HPP
#define SPIRE_INTEGER_TYPE_HPP
#include "Spire/StyleParser/DataTypes/DataType.hpp"

namespace Spire {

  /** Represents the integer type. */
  class IntegerType final : public DataType {
    public:

      /** Constructs an IntegerType. */
      IntegerType();

    protected:
      bool is_equal(const DataType& rhs) const override;
  };
}

#endif
