#include "Spire/Canvas/Records/RecordParser.hpp"
#include <Beam/Parsers/SequenceParser.hpp>
#include <Beam/Parsers/Types.hpp>
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include "Spire/Canvas/Types/ParserTypes.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/Spire/UserProfile.hpp"

using namespace Beam;
using namespace Beam::Parsers;
using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace std;

namespace {
  struct ParserBuilder {
    template<typename T>
    static RuleParser<Record::Field> Template(const NativeType& type,
        RefType<UserProfile> userProfile) {
      RuleParser<Record::Field> parser;
      parser = typename ParserType<T>::type();
      return parser;
    }

    template<>
    static RuleParser<Record::Field> Template<Security>(const NativeType& type,
        RefType<UserProfile> userProfile) {
      RuleParser<Record::Field> parser;
      parser = ParserType<Security>::type(userProfile->GetMarketDatabase());
      return parser;
    }

    template<>
    static RuleParser<Record::Field> Template<Record>(const NativeType& type,
        RefType<UserProfile> userProfile) {
      RuleParser<Record::Field> parser;
      parser = ParserType<Record>::type(static_cast<const RecordType&>(type),
        Ref(userProfile));
      return parser;
    }

    typedef boost::mpl::list<bool, Quantity, double, string, Security, Side,
      Record> SupportedTypes;
  };
}

RecordParser::RecordParser(const RecordType& recordType,
    RefType<UserProfile> userProfile) {
  vector<RuleParser<Record::Field>> fieldParserList;
  for(const auto& field : recordType.GetFields()) {
    fieldParserList.push_back(
      Instantiate<ParserBuilder>(field.m_type->GetNativeType())(*field.m_type,
      Ref(userProfile)));
  }
  auto fieldParser = Parsers::Sequence(fieldParserList, ',');
  auto recordParser = tokenize >> '(' >> fieldParser >> ')' >> ('\n' | eps_p);
  SetParser(recordParser);
}
