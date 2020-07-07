#ifndef SPIRE_NEGATION_TERM_HPP
#define SPIRE_NEGATION_TERM_HPP
#include <memory>
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  //! Evaluates to true iff the given term evaluates to false.
  class NegationTerm : public Term {
    public:

      //! Constructs a NegationTerm.
      /*!
        \param term The term to negate.
      */
      explicit NegationTerm(std::shared_ptr<Term> term);

      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const override;
  };
}

#endif
