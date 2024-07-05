#include <doctest/doctest.h>
#include "Spire/KeyBindings/KeywordFilteredTableModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("KeywordFilteredTableModel") {
  TEST_CASE("single_word_filter") {
    auto source = std::make_shared<ArrayTableModel>();
    auto keywords = std::make_shared<LocalTextModel>();
    auto table = KeywordFilteredTableModel(source, keywords);
  }
}
