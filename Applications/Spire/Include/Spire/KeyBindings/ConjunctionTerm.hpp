#ifndef SPIRE_CONJUNCTION_TERM_HPP
#define SPIRE_CONJUNCTION_TERM_HPP
#include <memory>
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  /*! \class ConjunctionTerm
      \brief Evaluates to true iff all given terms evaluate to true.
  */
  class ConjunctionTerm : public Term {
    public:

      //! Constructs a ConjunctionTerm.
      /*!
        \param terms The terms to evaluate for tags. Should contain at least
                     1 term.
      */
      explicit ConjunctionTerm(std::vector<std::shared_ptr<Term>> terms);

      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const override;
  };
}

#endif
