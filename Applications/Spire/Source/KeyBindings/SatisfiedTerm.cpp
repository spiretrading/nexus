#include "Spire/KeyBindings/SatisfiedTerm.hpp"

using namespace Spire;

bool SatisfiedTerm::is_satisfied(const std::vector<KeyBindings::Tag>&) const {
  return true;
}
