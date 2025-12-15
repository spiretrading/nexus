#include "Spire/Canvas/LuaNodes/RecordLuaReactorParameter.hpp"
#include <Beam/Utilities/Instantiate.hpp>
#include <boost/mpl/remove.hpp>
#include <boost/variant/get.hpp>
#include "Spire/Canvas/LuaNodes/LuaInterop.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/Types/RecordType.hpp"
#include "Spire/Canvas/Types/CanvasTypeRegistry.hpp"

using namespace Aspen;
using namespace Beam;
using namespace boost;
using namespace boost::mp11;
using namespace Spire;
using namespace std;

namespace {
  struct LuaValuePusher {
    using type = mp_remove<ValueTypes, Record>;

    template<typename T>
    void operator ()(lua_State& luaState, const Record::Field& value) const {
      PushLuaValue<T>()(luaState, boost::get<T>(value));
    }
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
        instantiate<LuaValuePusher>(field.m_type->GetNativeType())(
          state, value);
      }
      lua_settable(&state, -3);
    }
  }
}

RecordLuaReactorParameter::RecordLuaReactorParameter(Box<Record> reactor,
    const RecordType& recordType)
  : m_reactor(std::move(reactor)),
    m_recordType(recordType) {}

void RecordLuaReactorParameter::Push(lua_State& state) const {
  PushRecord(state, m_reactor.eval(), *m_recordType);
}

State RecordLuaReactorParameter::commit(int sequence) noexcept {
  return m_reactor.commit(sequence);
}

void RecordLuaReactorParameter::eval() const noexcept {
  m_reactor.eval();
}
