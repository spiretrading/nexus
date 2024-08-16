#ifndef SPIRE_COMPARISONSIGNATURES_HPP
#define SPIRE_COMPARISONSIGNATURES_HPP
#include <boost/mpl/end.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/transform.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

namespace Spire {

  //! Specifies the signatures used to test two values for equality.
  struct EqualitySignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T, bool>::type type;
    };

    typedef boost::mpl::transform<NativeTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type UniformTypes;

    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, double, bool>,
      boost::mpl::vector<double, Nexus::Quantity, bool>> MixedTypes;

    typedef boost::mpl::insert_range<UniformTypes,
      boost::mpl::end<UniformTypes>::type, MixedTypes>::type type;
  };

  //! Specifies the signatures used to compare two values.
  struct ComparisonSignatures {
    template<typename T>
    struct MakeSignature {
      typedef typename boost::mpl::vector<T, T, bool>::type type;
    };

    typedef boost::mpl::transform<ComparableTypes,
      MakeSignature<boost::mpl::placeholders::_1>>::type UniformTypes;

    typedef boost::mpl::list<
      boost::mpl::vector<Nexus::Quantity, double, bool>,
      boost::mpl::vector<double, Nexus::Quantity, bool>> MixedTypes;

    typedef boost::mpl::insert_range<UniformTypes,
      boost::mpl::end<UniformTypes>::type, MixedTypes>::type type;
  };
}

#endif
