#ifndef SPIRE_MATHSIGNATURES_HPP
#define SPIRE_MATHSIGNATURES_HPP
#include <boost/mp11.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies the signatures used to round values.
  struct RoundingNodeSignatures {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mp11::mp_list<double, double, double>,
      boost::mp11::mp_list<Nexus::Money, Nexus::Money, Nexus::Money>>;
  };

  //! Specifies the signatures used to get a type's extreme values.
  struct ExtremaNodeSignatures {
    template<typename T>
    using MakeSignature = boost::mp11::mp_list<T, T, T>;

    using type = boost::mp11::mp_transform<MakeSignature, ComparableTypes>;
  };
}

#endif
