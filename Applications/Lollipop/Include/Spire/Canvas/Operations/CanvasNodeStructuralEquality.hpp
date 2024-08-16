#ifndef SPIRE_CANVASNODESTRUCTURALEQUALITY_HPP
#define SPIRE_CANVASNODESTRUCTURALEQUALITY_HPP
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Tests two CanvasNodes for structural equality.
  /*!
    \param a One of the CanvasNodes to test.
    \param b The other CanvasNode to test.
    \return <code>true</code> iff <i>a</i> has the same structure as <i>b</i>.
  */
  bool IsStructurallyEqual(const CanvasNode& a, const CanvasNode& b);
}

#endif
