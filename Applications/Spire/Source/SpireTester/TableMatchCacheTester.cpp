#include <doctest/doctest.h>
#include "Spire/KeyBindings/TableMatchCache.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"

using namespace Spire;

namespace {
  bool matches(const QString& value, const QString& query) {
    return value.toLower().startsWith(query.toLower());
  }

  bool matches(int value, const QString& query) {
    return QString("%1").arg(value).startsWith(query);
  }
}

TEST_SUITE("TableMatchCache") {
  TEST_CASE("match_cache") {
    auto call_first_matcher = false;
    auto call_second_matcher = false;
    auto reset_call_matcher = [&] {
      call_first_matcher = false;
      call_second_matcher = false;
    };
    auto is_hit_first_cache = [&] {
      return !call_first_matcher;
    };
    auto is_hit_second_cache = [&] {
      return !call_second_matcher;
    };
    auto table = std::make_shared<ArrayTableModel>();
    table->push(std::vector<std::any>{1, QString("hello")});
    table->push(std::vector<std::any>{2, QString("world")});
    auto match_cache = TableMatchCache(table,
      [&] (const std::shared_ptr<TableModel>& table, int row, int column) {
        return [=, &call_first_matcher, &call_second_matcher]
            (const QString& query) {
          if(column == 0) {
            call_first_matcher = true;
            return matches(table->get<int>(row, column), query);
          }
          call_second_matcher = true;
          return matches(table->get<QString>(row, column), query);
        };
      });
    REQUIRE(match_cache.matches(0, "1"));
    REQUIRE(!is_hit_first_cache());
    reset_call_matcher();
    REQUIRE(match_cache.matches(0, "1"));
    REQUIRE(is_hit_first_cache());
    reset_call_matcher();
    REQUIRE(match_cache.matches(0, "1"));
    REQUIRE(is_hit_first_cache());
    reset_call_matcher();
    REQUIRE(match_cache.matches(0, "he"));
    REQUIRE(!is_hit_second_cache());
    reset_call_matcher();
    REQUIRE(match_cache.matches(0, "hello"));
    REQUIRE(!is_hit_second_cache());
    reset_call_matcher();
    REQUIRE(match_cache.matches(0, "hello"));
    REQUIRE(is_hit_second_cache());
    REQUIRE(match_cache.matches(1, "2"));
    REQUIRE(match_cache.matches(1, "wor"));
    REQUIRE(!match_cache.matches(0, "2"));
    REQUIRE(!match_cache.matches(1, "hello"));
  }
}
