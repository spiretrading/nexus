#include <Beam/Queries/StandardValues.hpp>
#include <doctest/doctest.h>
#include "Nexus/Queries/SqlTranslator.hpp"
#include "Nexus/OrderExecutionService/StandardQueries.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("SqlTranslator") {
  TEST_CASE("query_order_fields") {
    auto info_parameter_expression = ParameterExpression(0, typeid(OrderInfo));
    auto fields_access_expression = MemberAccessExpression(
      "fields", typeid(OrderFields), info_parameter_expression);
    auto security_access_expression = MemberAccessExpression(
      "security", typeid(Security), fields_access_expression);
    auto venue_access_expression = MemberAccessExpression(
      "venue", typeid(std::string), security_access_expression);
    auto equal_expression = "XTSX" == venue_access_expression;
    auto translator = Nexus::SqlTranslator("submissions", equal_expression);
    auto translation = translator.make();
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query == "(\"XTSX\" = venue)");
  }

  TEST_CASE("query_order_ids") {
    auto ids = std::vector<OrderId>();
    ids.push_back(13);
    ids.push_back(31);
    auto expression = make_order_id_filter(ids);
    auto translator = Nexus::SqlTranslator("submissions", expression);
    auto translation = translator.make();
    auto query = std::string();
    translation.append_query(query);
    REQUIRE(query ==
      "((submissions.order_id = 13) OR (submissions.order_id = 31))");
  }
}
