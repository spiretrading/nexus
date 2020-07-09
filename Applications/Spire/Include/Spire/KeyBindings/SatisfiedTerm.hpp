#ifndef SPIRE_SATISFIED_TERM_HPP
#define SPIRE_SATISFIED_TERM_HPP
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  //! Is satisfied for any list of tags.
  class SatisfiedTerm : public Term {
  public:
    bool is_satisfied(const std::vector<KeyBindings::Tag>& tags) const
      override;
  };
}

#endif
