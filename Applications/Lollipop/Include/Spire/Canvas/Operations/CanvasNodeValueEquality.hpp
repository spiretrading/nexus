#ifndef SPIRE_CANVASNODEVALUEEQUALITY_HPP
#define SPIRE_CANVASNODEVALUEEQUALITY_HPP
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  //! Tests two CanvasNodes for value equality.
  /*!
    \param a One of the CanvasNodes to test.
    \param b The other CanvasNode to test.
    \return <code>true</code> iff <i>a</i> has the same value as <i>b</i>.
  */
  bool IsValueEqual(const CanvasNode& a, const CanvasNode& b);
}

#endif
