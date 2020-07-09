#include "Spire/KeyBindings/DisjunctionTerm.hpp"
#include <algorithm>

using namespace Spire;

DisjunctionTerm::DisjunctionTerm(std::vector<std::shared_ptr<Term>> terms)
  : m_terms(std::move(terms)) {}

bool DisjunctionTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  return std::any_of(m_terms.begin(), m_terms.end(), [&] (auto& term) {
    return term->is_satisfied(tags);
  });
}
