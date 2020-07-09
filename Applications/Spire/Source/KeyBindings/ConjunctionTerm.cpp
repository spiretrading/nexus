#include "Spire/KeyBindings/ConjunctionTerm.hpp"
#include <algorithm>

using namespace Spire;

ConjunctionTerm::ConjunctionTerm(std::vector<std::shared_ptr<Term>> terms)
  : m_terms(std::move(terms)) {}

bool ConjunctionTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  return std::all_of(m_terms.begin(), m_terms.end(), [&] (auto& term) {
    return term->is_satisfied(tags);
  });
}
