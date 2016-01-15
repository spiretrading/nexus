#ifndef NEXUS_EXPRESSIONVISITOR_HPP
#define NEXUS_EXPRESSIONVISITOR_HPP
#include <boost/noncopyable.hpp>
#include "Nexus/Queries/Queries.hpp"

namespace Nexus {
namespace Queries {

  /*! \class ExpressionVisitor
      \brief Implements the visitor pattern for Expressions.
   */
  class ExpressionVisitor : private boost::noncopyable {
    public:
      virtual ~ExpressionVisitor() = default;
  };
}
}

#endif
