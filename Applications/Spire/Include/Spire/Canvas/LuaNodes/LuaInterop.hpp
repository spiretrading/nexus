#ifndef SPIRE_LUAINTEROP_HPP
#define SPIRE_LUAINTEROP_HPP
#include <Beam/Reactors/LuaReactor.hpp>
#include <Beam/Reactors/NativeLuaReactorParameter.hpp>
#include <Beam/Tasks/Task.hpp>
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/OrderStatus.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Canvas/Records/Record.hpp"

namespace Beam {
namespace Reactors {
  template<>
  struct PopLuaValue<Nexus::CurrencyId> {
    Nexus::CurrencyId operator ()(lua_State& state) const {
      return Nexus::CurrencyId{static_cast<int>(lua_tonumber(&state, -1))};
    }
  };

  template<>
  struct PushLuaValue<Nexus::CurrencyId> {
    void operator ()(lua_State& state, Nexus::CurrencyId value) const {
      lua_pushnumber(&state, static_cast<int>(value.m_value));
    }
  };

  template<>
  struct PopLuaValue<Nexus::Money> {
    Nexus::Money operator ()(lua_State& state) const {
      return Nexus::Money{Nexus::Quantity::FromRepresentation(
        lua_tonumber(&state, -1))};
    }
  };

  template<>
  struct PushLuaValue<Nexus::Money> {
    void operator ()(lua_State& state, Nexus::Money value) const {
      lua_pushnumber(&state,
        static_cast<Nexus::Quantity>(value).GetRepresentation());
    }
  };

  template<>
  struct PopLuaValue<Nexus::Quantity> {
    Nexus::Quantity operator ()(lua_State& state) const {
      return Nexus::Quantity::FromRepresentation(lua_tonumber(&state, -1));
    }
  };

  template<>
  struct PushLuaValue<Nexus::Quantity> {
    void operator ()(lua_State& state, Nexus::Quantity value) const {
      lua_pushnumber(&state,
        static_cast<Nexus::Quantity>(value).GetRepresentation());
    }
  };

  template<>
  struct PopLuaValue<Nexus::Security> {
    Nexus::Security operator ()(lua_State& state) const {

      // TODO
      return Nexus::Security();
    }
  };

  template<>
  struct PushLuaValue<Nexus::Security> {
    void operator ()(lua_State& state, const Nexus::Security& value) const {

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
  struct PopLuaValue<Spire::Record> {
    Spire::Record operator ()(lua_State& state) const {

      // TODO
      return Spire::Record();
    }
  };
}
}

namespace Spire {
  inline void ExportOrderStatus(lua_State& state) {
    lua_newtable(&state);
    lua_pushliteral(&state, "NONE");
    lua_pushnumber(&state, Nexus::OrderStatus::NONE);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "PENDING_NEW");
    lua_pushnumber(&state, Nexus::OrderStatus::PENDING_NEW);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "REJECTED");
    lua_pushnumber(&state, Nexus::OrderStatus::REJECTED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "NEW");
    lua_pushnumber(&state, Nexus::OrderStatus::NEW);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "PARTIALLY_FILLED");
    lua_pushnumber(&state, Nexus::OrderStatus::PARTIALLY_FILLED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "EXPIRED");
    lua_pushnumber(&state, Nexus::OrderStatus::EXPIRED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "CANCELED");
    lua_pushnumber(&state, Nexus::OrderStatus::CANCELED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "SUSPENDED");
    lua_pushnumber(&state, Nexus::OrderStatus::SUSPENDED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "STOPPED");
    lua_pushnumber(&state, Nexus::OrderStatus::STOPPED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "FILLED");
    lua_pushnumber(&state, Nexus::OrderStatus::FILLED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "DONE_FOR_DAY");
    lua_pushnumber(&state, Nexus::OrderStatus::DONE_FOR_DAY);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "PENDING_CANCEL");
    lua_pushnumber(&state, Nexus::OrderStatus::PENDING_CANCEL);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "CANCEL_REJECT");
    lua_pushnumber(&state, Nexus::OrderStatus::CANCEL_REJECT);
    lua_settable(&state, -3);
    lua_setglobal(&state, "OrderStatus");
  }

  inline void ExportOrderType(lua_State& state) {
    lua_newtable(&state);
    lua_pushliteral(&state, "NONE");
    lua_pushnumber(&state, Nexus::OrderType::NONE);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "MARKET");
    lua_pushnumber(&state, Nexus::OrderType::MARKET);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "LIMIT");
    lua_pushnumber(&state, Nexus::OrderType::LIMIT);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "PEGGED");
    lua_pushnumber(&state, Nexus::OrderType::PEGGED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "STOP");
    lua_pushnumber(&state, Nexus::OrderType::STOP);
    lua_settable(&state, -3);
    lua_setglobal(&state, "OrderType");
  }

  inline void ExportSide(lua_State& state) {
    lua_newtable(&state);
    lua_pushliteral(&state, "NONE");
    lua_pushnumber(&state, Nexus::Side::NONE);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "BID");
    lua_pushnumber(&state, Nexus::Side::BID);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "ASK");
    lua_pushnumber(&state, Nexus::Side::ASK);
    lua_settable(&state, -3);
    lua_setglobal(&state, "Side");
  }

  inline void ExportTaskState(lua_State& state) {
    lua_newtable(&state);
    lua_pushliteral(&state, "NONE");
    lua_pushnumber(&state, Beam::Tasks::Task::State::NONE);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "INITIALIZING");
    lua_pushnumber(&state, Beam::Tasks::Task::State::INITIALIZING);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "ACTIVE");
    lua_pushnumber(&state, Beam::Tasks::Task::State::ACTIVE);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "PENDING_CANCEL");
    lua_pushnumber(&state, Beam::Tasks::Task::State::PENDING_CANCEL);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "CANCELED");
    lua_pushnumber(&state, Beam::Tasks::Task::State::CANCELED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "FAILED");
    lua_pushnumber(&state, Beam::Tasks::Task::State::FAILED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "EXPIRED");
    lua_pushnumber(&state, Beam::Tasks::Task::State::EXPIRED);
    lua_settable(&state, -3);
    lua_pushliteral(&state, "COMPLETE");
    lua_pushnumber(&state, Beam::Tasks::Task::State::COMPLETE);
    lua_settable(&state, -3);
    lua_setglobal(&state, "TaskState");
  }

  inline void ExportLuaTypes(lua_State& state) {
    ExportOrderStatus(state);
    ExportOrderType(state);
    ExportSide(state);
    ExportTaskState(state);
  }
}

#endif
