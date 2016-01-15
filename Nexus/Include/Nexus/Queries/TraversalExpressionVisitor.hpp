#ifndef NEXUS_TRAVERSALEXPRESSIONVISITOR_HPP
#define NEXUS_TRAVERSALEXPRESSIONVISITOR_HPP
#include <Beam/Queries/TraversalExpressionVisitor.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Queries/ExpressionVisitor.hpp"
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace Queries {

  /*! \class TraversalExpressionVisitor
      \brief An ExpressionVisitor that traverses all of its children.
   */
  class TraversalExpressionVisitor :
      public Beam::Queries::TraversalExpressionVisitor,
      public ExpressionVisitor {
    public:
  };
}
}

#endif
