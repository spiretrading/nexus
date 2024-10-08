#ifndef NEXUS_EXPRESSION_VISITOR_HPP
#define NEXUS_EXPRESSION_VISITOR_HPP
#include "Nexus/Queries/Queries.hpp"

namespace Nexus::Queries {

  /** Implements the visitor pattern for Expressions. */
  class ExpressionVisitor {
    public:
      virtual ~ExpressionVisitor() = default;

    protected:

      /** Constructs an ExpressionVisitor. */
      ExpressionVisitor() = default;

    private:
      ExpressionVisitor(const ExpressionVisitor&) = delete;
      ExpressionVisitor& operator =(const ExpressionVisitor&) = delete;
  };
}

#endif
