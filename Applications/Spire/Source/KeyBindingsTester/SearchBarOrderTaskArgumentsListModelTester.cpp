#include <doctest/doctest.h>
#include "Nexus/Definitions/StandardVenues.hpp"
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
      get_default_additional_tag_database());
    REQUIRE(search_list.get_size() == 3);
    keywords->set("Q");
    REQUIRE(search_list.get_size() == 0);
    keywords->set("QQ");
    REQUIRE(search_list.get_size() == 0);
    keywords->set("");
    REQUIRE(search_list.get_size() == 3);
  }

  TEST_CASE("scope_venue_filter") {
    auto source = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    auto task1 = OrderTaskArguments();
    task1.m_name = "Alpha Limit Bid";
    task1.m_scope = Scope(Venues::TSX);
    source->push(task1);
    auto task2 = OrderTaskArguments();
    task2.m_name = "Beta Limit Bid";
    task2.m_scope = Scope(Venues::ASX);
    source->push(task2);
    auto keywords = std::make_shared<LocalTextModel>();
    auto search_list = SearchBarOrderTaskArgumentsListModel(
      source, keywords, get_default_additional_tag_database());
    REQUIRE(search_list.get_size() == 2);
    keywords->set("tsx");
    REQUIRE(search_list.get_size() == 1);
    REQUIRE(search_list.get(0).m_name == "Alpha Limit Bid");
    keywords->set("");
    REQUIRE(search_list.get_size() == 2);
  }

  TEST_CASE("revalidate_after_insert") {
    auto source = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    auto alpha = OrderTaskArguments();
    alpha.m_name = "Alpha";
    alpha.m_scope = Scope(Venues::TSX);
    source->push(alpha);
    auto keywords = std::make_shared<LocalTextModel>();
    auto search_list = SearchBarOrderTaskArgumentsListModel(
      source, keywords, get_default_additional_tag_database());
    keywords->set("tsx");
    REQUIRE(search_list.get_size() == 1);
    auto beta = OrderTaskArguments();
    beta.m_name = "Beta";
    beta.m_scope = Scope(Venues::ASX);
    source->insert(beta, 0);
    REQUIRE(search_list.get_size() == 1);
    REQUIRE(search_list.get(0).m_name == "Alpha");
  }

  TEST_CASE("revalidate_after_remove") {
    auto source = std::make_shared<ArrayListModel<OrderTaskArguments>>();
    auto alpha = OrderTaskArguments();
    alpha.m_name = "Alpha";
    alpha.m_scope = Scope(Venues::TSX);
    source->push(alpha);
    auto beta = OrderTaskArguments();
    beta.m_name = "Beta";
    beta.m_scope = Scope(Venues::ASX);
    source->push(beta);
    auto gamma = OrderTaskArguments();
    gamma.m_name = "Gamma";
    gamma.m_scope = Scope(Venues::TSX);
    source->push(gamma);
    auto keywords = std::make_shared<LocalTextModel>();
    auto search_list = SearchBarOrderTaskArgumentsListModel(
      source, keywords, get_default_additional_tag_database());
    keywords->set("tsx");
    REQUIRE(search_list.get_size() == 2);
    source->remove(0);
    keywords->set("");
    keywords->set("tsx");
    REQUIRE(search_list.get_size() == 1);
    REQUIRE(search_list.get(0).m_name == "Gamma");
  }
}
