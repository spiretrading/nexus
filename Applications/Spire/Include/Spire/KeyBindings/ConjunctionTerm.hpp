#ifndef SPIRE_CONJUNCTION_TERM_HPP
#define SPIRE_CONJUNCTION_TERM_HPP
#include <memory>
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  //! Is satisfied iff there is no term in a list that evaluates to false.
  class ConjunctionTerm : public Term {
    public:

      //! Constructs a ConjunctionTerm.
      /*!
        \param terms The terms to evaluate for tags.
      */
      explicit ConjunctionTerm(std::vector<std::shared_ptr<Term>> terms);

      bool is_satisfied(const std::vector<KeyBindings::Tag>& tags) const
        override;

    private:
      std::vector<std::shared_ptr<Term>> m_terms;
  };
}

#endif
