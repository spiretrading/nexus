#ifndef NEXUS_TRAVERSAL_EXPRESSION_VISITOR_HPP
#define NEXUS_TRAVERSAL_EXPRESSION_VISITOR_HPP
#include <Beam/Queries/TraversalExpressionVisitor.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::Queries {

  /** An ExpressionVisitor that traverses all of its children. */
  class TraversalExpressionVisitor :
    public Beam::Queries::TraversalExpressionVisitor,
    public ExpressionVisitor {};
}

#endif
