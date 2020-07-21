#include "Spire/KeyBindings/NegationTerm.hpp"

using namespace Spire;

NegationTerm::NegationTerm(std::shared_ptr<Term> term)
  : m_term(std::move(term)) {}

bool NegationTerm::is_satisfied(const std::vector<KeyBindings::Tag>&
    tags) const {
  return !(m_term->is_satisfied(tags));
}
