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
    auto keywords = std::make_shared<LocalTextModel>();
    auto table = SearchBarOrderTaskArgumentsListModel(source, keywords,
      GetDefaultCountryDatabase(), GetDefaultMarketDatabase(),
      GetDefaultDestinationDatabase());
  }
}
