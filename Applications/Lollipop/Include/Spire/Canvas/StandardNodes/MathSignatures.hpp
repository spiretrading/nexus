#ifndef SPIRE_MATHSIGNATURES_HPP
#define SPIRE_MATHSIGNATURES_HPP
#include <boost/mpl/end.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/transform.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies the signatures used to round values.
  struct RoundingNodeSignatures {
    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, Nexus::Quantity, Nexus::Quantity>,
      boost::mpl::vector<double, Nexus::Quantity, double>,
      boost::mpl::vector<Nexus::Money, Nexus::Quantity, Nexus::Money>> type;
  };

  //! Specifies the signatures used to get a type's extreme values.
  struct ExtremaNodeSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T, T>::type type;
    };

    typedef boost::mpl::transform<ComparableTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type type;
  };
}

#endif
