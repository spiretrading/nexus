#include "Spire/Canvas/LuaNodes/RecordLuaReactorParameter.hpp"
#include <Beam/Utilities/InstantiateTemplate.hpp>
#include <boost/mpl/remove.hpp>
#include <boost/variant/get.hpp>
#include "Spire/Canvas/LuaNodes/LuaInterop.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

using namespace Beam;
using namespace Beam::Reactors;
using namespace Spire;
using namespace std;

namespace {
  struct LuaValuePusher {
    template<typename T>
    static void Template(lua_State& luaState, const Record::Field& value) {
      PushLuaValue<T>()(luaState, boost::get<T>(value));
    }

    typedef boost::mpl::remove<ValueTypes, Record>::type SupportedTypes;
  };

  void PushRecord(lua_State& state, const Record& record,
      const RecordType& type) {
    lua_newtable(&state);
    for(size_t i = 0; i < type.GetFields().size(); ++i) {
      auto& field = type.GetFields()[i];
      auto& value = record.GetFields()[i];
      lua_pushstring(&state, field.m_name.c_str());
      if(field.m_type->GetCompatibility(RecordType::GetEmptyRecordType()) ==
          CanvasType::Compatibility::EQUAL) {
        PushRecord(state, boost::get<Record>(value),
          static_cast<const RecordType&>(*field.m_type));
      } else {
        Instantiate<LuaValuePusher>(field.m_type->GetNativeType())(state,
          value);
      }
      lua_settable(&state, -3);
    }
  }
}

RecordLuaReactorParameter::RecordLuaReactorParameter(
    std::shared_ptr<Reactor<Record>> reactor, const RecordType& recordType)
    : LuaReactorParameter(std::move(reactor)),
      m_recordType(recordType) {}

void RecordLuaReactorParameter::Push(lua_State& state) const {
  PushRecord(state, static_cast<const Reactor<Record>&>(GetReactor()).Eval(),
    *m_recordType);
}
