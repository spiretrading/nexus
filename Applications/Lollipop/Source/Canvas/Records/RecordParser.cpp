#include "Spire/Canvas/Records/RecordParser.hpp"
#include <Beam/Parsers/RuleParser.hpp>
#include <Beam/Parsers/SequenceParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include "Nexus/Parsers/CountryParser.hpp"
#include "Nexus/Parsers/CurrencyParser.hpp"
#include "Nexus/Parsers/MarketParser.hpp"
#include "Nexus/Parsers/MoneyParser.hpp"
#include "Nexus/Parsers/OrderStatusParser.hpp"
#include "Nexus/Parsers/OrderTypeParser.hpp"
#include "Nexus/Parsers/QuantityParser.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"
#include "Nexus/Parsers/SideParser.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  struct ParserBuilder {
    template<typename T>
    static RuleParser<Record::Field> Template(const NativeType& type,
        Ref<UserProfile> userProfile) {
      auto parser = RuleParser<Record::Field>();
      parser.SetRule(default_parser<T>);
      return parser;
    }

    template<>
    static RuleParser<Record::Field> Template<Security>(const NativeType& type,
        Ref<UserProfile> userProfile) {
      auto parser = RuleParser<Record::Field>();
      parser.SetRule(SecurityParser(userProfile->GetMarketDatabase()));
      return parser;
    }

    template<>
    static RuleParser<Record::Field> Template<Record>(const NativeType& type,
        Ref<UserProfile> userProfile) {
      auto parser = RuleParser<Record::Field>();
      parser.SetRule(RecordParser(static_cast<const RecordType&>(type),
        Ref(userProfile)));
      return parser;
    }

    typedef boost::mpl::list<bool, Quantity, double, std::string, Security,
      Side, Record> SupportedTypes;
  };
}

RuleParser<Record> Spire::RecordParser(const RecordType& recordType,
    Ref<UserProfile> userProfile) {
  auto fieldParserList = std::vector<RuleParser<Record::Field>>();
  for(auto& field : recordType.GetFields()) {
    fieldParserList.push_back(Instantiate<ParserBuilder>(
      field.m_type->GetNativeType())(*field.m_type, Ref(userProfile)));
  }
  auto fieldParser = Parsers::Sequence(fieldParserList, ',');
  return tokenize >> '(' >> fieldParser >> ')' >> ('\n' | eps_p);
}
