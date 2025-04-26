#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/KeyBindings/SearchBarOrderTaskArgumentsListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("SearchBarOrderTaskArgumentsListModel") {
  TEST_CASE("single_word_filter") {
    auto source = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    auto task1 = OrderTaskArguments();
    task1.m_name = "Foo";
    source->push(task1);
    auto task2 = OrderTaskArguments();
    task2.m_name = "Bar";
    source->push(task2);
    auto task3 = OrderTaskArguments();
    task3.m_name = "Baz";
    source->push(task3);
    auto keywords = std::make_shared<LocalTextModel>();
    auto search_list = SearchBarOrderTaskArgumentsListModel(source, keywords,
      GetDefaultCountryDatabase(), GetDefaultMarketDatabase(),
      GetDefaultDestinationDatabase(), get_default_additional_tag_database());
    REQUIRE(search_list.get_size() == 3);
    keywords->set("Q");
    REQUIRE(search_list.get_size() == 0);
    keywords->set("QQ");
    REQUIRE(search_list.get_size() == 0);
    keywords->set("");
    REQUIRE(search_list.get_size() == 3);
  }
}
