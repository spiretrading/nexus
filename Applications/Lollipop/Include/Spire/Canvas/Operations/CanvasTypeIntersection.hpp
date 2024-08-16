#ifndef SPIRE_CANVASTYPEINTERSECTION_HPP
#define SPIRE_CANVASTYPEINTERSECTION_HPP
#include <memory>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Returns the intersection of two CanvasTypes.
  /*!
    \param a A CanvasType.
    \param b A CanvasType.
    \return A CanvasType containing the common CanvasTypes in <i>a</i> and
            <i>b</i>.
  */
  std::shared_ptr<CanvasType> Intersect(const CanvasType& a,
    const CanvasType& b);

  //! Returns the intersection of two CanvasTypes, if the intersection is empty,
  //! then returns the <i>left</i> parameter.
  /*!
    \param left A CanvasType.
    \param right A CanvasType.
    \return A CanvasType containing the common CanvasTypes in <i>left</i>
            and <i>right</i> or <i>left</i> iff there are no common types.
  */
  std::shared_ptr<CanvasType> LeftIntersect(const CanvasType& left,
    const CanvasType& right);
}

#endif
