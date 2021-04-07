#include "Spire/Styles/VoidSelector.hpp"

using namespace Spire;
using namespace Spire::Styles;

bool VoidSelector::is_match(const VoidSelector& selector) const {
  return false;
}

std::vector<QWidget*> Spire::Styles::select(
    const VoidSelector& selector, QWidget& source) {
  return {};
}
