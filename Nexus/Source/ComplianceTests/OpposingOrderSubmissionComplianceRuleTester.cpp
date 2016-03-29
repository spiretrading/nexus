#include "Nexus/ComplianceTests/OpposingOrderSubmissionComplianceRuleTester.hpp"
#include <Beam/TimeService/FixedTimeClient.hpp>
#include "Nexus/Compliance/OpposingOrderSubmissionComplianceRule.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultCurrencyDatabase.hpp"
#include "Nexus/Definitions/DefaultDestinationDatabase.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Beam::TimeService;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;
using namespace std;
