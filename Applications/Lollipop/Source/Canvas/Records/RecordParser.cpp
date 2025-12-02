#include "Spire/Canvas/Records/RecordParser.hpp"
#include <Beam/Parsers/SequenceParser.hpp>
#include <Beam/Parsers/Tokenize.hpp>
#include <Beam/Utilities/Instantiate.hpp>
#include "Nexus/Parsers/CountryParser.hpp"
#include "Nexus/Parsers/CurrencyParser.hpp"
#include "Nexus/Parsers/MoneyParser.hpp"
#include "Nexus/Parsers/OrderStatusParser.hpp"
#include "Nexus/Parsers/OrderTypeParser.hpp"
#include "Nexus/Parsers/QuantityParser.hpp"
#include "Nexus/Parsers/SecurityParser.hpp"
#include "Nexus/Parsers/SideParser.hpp"
#include "Nexus/Parsers/VenueParser.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"
#include "Spire/UI/UserProfile.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::mp11;
using namespace Nexus;
using namespace Spire;

namespace {
  struct ParserBuilder {
    using type =
      mp_list<bool, Quantity, double, std::string, Security, Side, Record>;

    template<typename T>
    Parser<Record::Field> operator ()(const NativeType& type,
        Ref<UserProfile> userProfile) const {
      return cast<Record::Field>(default_parser<T>);
    }

    template<>
    Parser<Record::Field> operator ()<Security>(const NativeType& type,
        Ref<UserProfile> userProfile) const {
      return cast<Record::Field>(
        SecurityParser(userProfile->GetVenueDatabase()));
    }

    template<>
    Parser<Record::Field> operator ()<Record>(const NativeType& type,
        Ref<UserProfile> userProfile) const {
      return cast<Record::Field>(RecordParser(
        static_cast<const RecordType&>(type), Ref(userProfile)));
    }
  };
}

Parser<Record> Spire::RecordParser(const RecordType& recordType,
    Ref<UserProfile> userProfile) {
  auto fieldParserList = std::vector<Parser<Record::Field>>();
  for(auto& field : recordType.GetFields()) {
    fieldParserList.push_back(instantiate<ParserBuilder>(
      field.m_type->GetNativeType())(*field.m_type, Ref(userProfile)));
  }
  auto fieldParser = sequence(fieldParserList, ',');
  return cast<Record>(tokenize('(', fieldParser, ')', ('\n' | eps_p)));
}
