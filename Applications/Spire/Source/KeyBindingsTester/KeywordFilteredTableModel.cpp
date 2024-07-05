#include <doctest/doctest.h>
#include "Spire/KeyBindings/KeywordFilteredTableModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std::literals;

namespace {
  void require_equal(const TableModel& table,
      std::vector<std::vector<std::string>> expected) {
    REQUIRE(table.get_row_size() == expected.size());
    for(auto i = 0; i != table.get_row_size(); ++i) {
      REQUIRE(table.get_column_size() == expected[i].size());
      for(auto j = 0; j != table.get_column_size(); ++j) {
        REQUIRE(table.get<std::string>(i, j) == expected[i][j]);
      }
    }
  }
}

TEST_SUITE("KeywordFilteredTableModel") {
  TEST_CASE("single_word_filter") {
    auto source = std::make_shared<ArrayTableModel>();
    source->push({"A"s, "B"s});
    source->push({"C"s, "D"s});
    source->push({"E"s, "F"s});
    auto keywords = std::make_shared<LocalTextModel>();
    auto table = KeywordFilteredTableModel(source, keywords);
    require_equal(table, {{"A", "B"}, {"C", "D"}, {"E", "F"}});
    keywords->set("A");
    require_equal(table, {{"A", "B"}});
  }
}
