#include "Nexus/QueriesTests/SqlTranslatorTester.hpp"
#include <Beam/Queries/StandardValues.hpp>
#include "Nexus/Queries/SqlTranslator.hpp"

using namespace Nexus;
using namespace Nexus::Queries;
using namespace Nexus::Queries::Tests;

void SqlTranslatorTester::TestQueryOrderFields() {
  Beam::Queries::ParameterExpression infoParameterExpression(
    0, Nexus::Queries::OrderInfoType());
  Beam::Queries::MemberAccessExpression fieldsAccessExpression("fields",
    Nexus::Queries::OrderFieldsType(), infoParameterExpression);
  Beam::Queries::MemberAccessExpression securityAccessExpression("security",
    Nexus::Queries::SecurityType(), fieldsAccessExpression);
  Beam::Queries::MemberAccessExpression marketAccessExpression("market",
    Beam::Queries::StringType(), securityAccessExpression);
  Beam::Queries::StringValue queryMarketCode("XTSX");
  Beam::Queries::ConstantExpression marketCodeExpression(queryMarketCode);
  auto equalExpression = Beam::Queries::MakeEqualsExpression(
    marketCodeExpression, marketAccessExpression);
  SqlTranslator translator("submissions", equalExpression);
  auto translation = translator.Build();
  std::string query;
  translation.append_query(query);
  CPPUNIT_ASSERT(query == "\"XTSX\" = market");
}
