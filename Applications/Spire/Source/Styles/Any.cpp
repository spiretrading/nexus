#include "Spire/Styles/Any.hpp"

using namespace Spire;
using namespace Spire::Styles;

bool Any::is_match(const Any& selector) const {
  return true;
}

std::vector<QWidget*> Spire::Styles::select(
    const Any& selector, QWidget& source) {
  return std::vector{&source};
}
