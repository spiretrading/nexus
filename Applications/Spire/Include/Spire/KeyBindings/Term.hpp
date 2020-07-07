#ifndef SPIRE_TERM_HPP
#define SPIRE_TERM_HPP
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  //! Evaluates a condition on a list of KeyBindings tags.
  class Term {
    public:
      virtual ~Term() = default;

      //! Tells whether the term's condition is satisfied for a list of tags.
      /*!
        \param tags The list of tags.
        \return True iff the term's condition is satisfied for the tags.
      */
      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const = 0;
  };
}

#endif
