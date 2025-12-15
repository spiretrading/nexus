#ifndef NEXUS_TRAVERSAL_EXPRESSION_VISITOR_HPP
#define NEXUS_TRAVERSAL_EXPRESSION_VISITOR_HPP
#include <Beam/Queries/TraversalExpressionVisitor.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"

namespace Nexus {

  /** An ExpressionVisitor that traverses all of its children. */
  class TraversalExpressionVisitor : public Beam::TraversalExpressionVisitor,
    public ExpressionVisitor {};
}

#endif
