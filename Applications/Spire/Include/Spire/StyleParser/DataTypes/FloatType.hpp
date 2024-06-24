#ifndef SPIRE_FLOAT_TYPE_HPP
#define SPIRE_FLOAT_TYPE_HPP
#include "Spire/StyleParser/DataTypes/DataType.hpp"

namespace Spire {

  /** Represents the floating-point type. */
  class FloatType : public DataType {
    public:

      /** Constructs a FloatType. */
      FloatType();

    protected:
      bool is_equal(const DataType& rhs) const override;
  };
}

#endif
