#ifndef SPIRE_LUAINTEROP_HPP
#define SPIRE_LUAINTEROP_HPP
#include "Nexus/Definitions/Asset.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/Ticker.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Nexus/Definitions/Venue.hpp"
#include "Spire/Canvas/Records/Record.hpp"
#include "Spire/Canvas/LuaNodes/LuaReactor.hpp"
#include "Spire/Canvas/LuaNodes/NativeLuaReactorParameter.hpp"

namespace Spire {
  template<>
  struct PopLuaValue<Nexus::Asset> {
    Nexus::Asset operator ()(lua_State& state) const {
      return Nexus::Asset(
        static_cast<Nexus::Asset::Id>(lua_tointeger(&state, -1)));
    }
  };

  template<>
  struct PushLuaValue<Nexus::Asset> {
    void operator ()(lua_State& state, Nexus::Asset value) const {
      lua_pushinteger(&state, value.get_id());
    }
  };

  template<>
  struct PopLuaValue<Nexus::CurrencyId> {
    Nexus::CurrencyId operator ()(lua_State& state) const {
      return Nexus::CurrencyId(static_cast<std::uint16_t>(
        lua_tonumber(&state, -1)));
    }
  };

  template<>
  struct PushLuaValue<Nexus::CurrencyId> {
    void operator ()(lua_State& state, Nexus::CurrencyId value) const {
      lua_pushnumber(&state, static_cast<std::uint16_t>(value));
    }
  };

  template<>
  struct PopLuaValue<Nexus::Money> {
    Nexus::Money operator ()(lua_State& state) const {
      return Nexus::Money{Nexus::Quantity::from_representation(
        lua_tonumber(&state, -1))};
    }
  };

  template<>
  struct PushLuaValue<Nexus::Money> {
    void operator ()(lua_State& state, Nexus::Money value) const {
      lua_pushnumber(&state,
        static_cast<Nexus::Quantity>(value).get_representation());
    }
  };

  template<>
  struct PopLuaValue<Nexus::Quantity> {
    Nexus::Quantity operator ()(lua_State& state) const {
      return Nexus::Quantity::from_representation(lua_tonumber(&state, -1));
    }
  };

  template<>
  struct PushLuaValue<Nexus::Quantity> {
    void operator ()(lua_State& state, Nexus::Quantity value) const {
      lua_pushnumber(&state,
        static_cast<Nexus::Quantity>(value).get_representation());
    }
  };

  template<>
  struct PopLuaValue<Nexus::Ticker> {
    Nexus::Ticker operator ()(lua_State& state) const {

      // TODO
      return Nexus::Ticker();
    }
  };

  template<>
  struct PushLuaValue<Nexus::Ticker> {
    void operator ()(lua_State& state, const Nexus::Ticker& value) const {

      // TODO
    }
  };

  template<>
  struct PopLuaValue<Nexus::TimeInForce> {
    Nexus::TimeInForce operator ()(lua_State& state) const {

      // TODO
      return Nexus::TimeInForce();
    }
  };

  template<>
  struct PushLuaValue<Nexus::TimeInForce> {
    void operator ()(lua_State& state, const Nexus::TimeInForce& value) const {

      // TODO
    }
  };

  template<>
  struct PopLuaValue<Beam::Range> {
    Beam::Range operator ()(lua_State& state) const {

      // TODO
      return Beam::Range::EMPTY;
    }
  };

  template<>
  struct PushLuaValue<Beam::Range> {
    void operator ()(lua_State& state,
        const Beam::Range& value) const {

      // TODO
    }
  };

  template<>
  struct PopLuaValue<Beam::Sequence> {
    Beam::Sequence operator ()(lua_State& state) const {

      // TODO
      return Beam::Sequence();
    }
  };

  template<>
  struct PushLuaValue<Beam::Sequence> {
    void operator ()(lua_State& state,
        const Beam::Sequence& value) const {

      // TODO
    }
  };

  template<>
  struct PopLuaValue<Spire::Record> {
    Spire::Record operator ()(lua_State& state) const {

      // TODO
      return Spire::Record();
    }
  };

  template<>
  struct PopLuaValue<Nexus::Venue> {
    Nexus::Venue operator ()(lua_State& state) const {

      // TODO
      return Nexus::Venue();
    }
  };

  template<>
  struct PushLuaValue<Nexus::Venue> {
    void operator ()(lua_State& state, Nexus::Venue value) const {

      // TODO
    }
  };

  void ExportOrderStatus(lua_State& state);
  void ExportOrderType(lua_State& state);
  void ExportSide(lua_State& state);
  void ExportTaskState(lua_State& state);
  void ExportLuaTypes(lua_State& state);
}

#endif
