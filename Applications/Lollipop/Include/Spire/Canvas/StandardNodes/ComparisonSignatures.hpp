#ifndef SPIRE_COMPARISONSIGNATURES_HPP
#define SPIRE_COMPARISONSIGNATURES_HPP
#include <boost/mp11.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies the signatures used to test two values for equality.
  struct EqualitySignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T, bool>;

    using UniformTypes = boost::mp11::mp_transform<MakeSignature, NativeTypes>;

    using MixedTypes = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, double, bool>,
      boost::mp11::mp_list<double, Nexus::Quantity, bool>>;

    using type = boost::mp11::mp_append<UniformTypes, MixedTypes>;
  };

  //! Specifies the signatures used to compare two values.
  struct ComparisonSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T, bool>;

    using UniformTypes =
      boost::mp11::mp_transform<MakeSignature, ComparableTypes>;

    using MixedTypes = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, double, bool>,
      boost::mp11::mp_list<double, Nexus::Quantity, bool>>;

    using type = boost::mp11::mp_append<UniformTypes, MixedTypes>;
  };
}

#endif
