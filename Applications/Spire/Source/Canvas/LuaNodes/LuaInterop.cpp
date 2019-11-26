#include "Spire/Canvas/LuaNodes/LuaInterop.hpp"

using namespace Nexus;
using namespace Spire;

void Spire::ExportOrderStatus(lua_State& state) {
  lua_newtable(&state);
  lua_pushliteral(&state, "NONE");
  lua_pushnumber(&state, OrderStatus::NONE);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "PENDING_NEW");
  lua_pushnumber(&state, OrderStatus::PENDING_NEW);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "REJECTED");
  lua_pushnumber(&state, OrderStatus::REJECTED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "NEW");
  lua_pushnumber(&state, OrderStatus::NEW);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "PARTIALLY_FILLED");
  lua_pushnumber(&state, OrderStatus::PARTIALLY_FILLED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "EXPIRED");
  lua_pushnumber(&state, OrderStatus::EXPIRED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "CANCELED");
  lua_pushnumber(&state, OrderStatus::CANCELED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "SUSPENDED");
  lua_pushnumber(&state, OrderStatus::SUSPENDED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "STOPPED");
  lua_pushnumber(&state, OrderStatus::STOPPED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "FILLED");
  lua_pushnumber(&state, OrderStatus::FILLED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "DONE_FOR_DAY");
  lua_pushnumber(&state, OrderStatus::DONE_FOR_DAY);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "PENDING_CANCEL");
  lua_pushnumber(&state, OrderStatus::PENDING_CANCEL);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "CANCEL_REJECT");
  lua_pushnumber(&state, OrderStatus::CANCEL_REJECT);
  lua_settable(&state, -3);
  lua_setglobal(&state, "OrderStatus");
}

void Spire::ExportOrderType(lua_State& state) {
  lua_newtable(&state);
  lua_pushliteral(&state, "NONE");
  lua_pushnumber(&state, OrderType::NONE);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "MARKET");
  lua_pushnumber(&state, OrderType::MARKET);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "LIMIT");
  lua_pushnumber(&state, OrderType::LIMIT);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "PEGGED");
  lua_pushnumber(&state, OrderType::PEGGED);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "STOP");
  lua_pushnumber(&state, OrderType::STOP);
  lua_settable(&state, -3);
  lua_setglobal(&state, "OrderType");
}

void Spire::ExportSide(lua_State& state) {
  lua_newtable(&state);
  lua_pushliteral(&state, "NONE");
  lua_pushnumber(&state, Side::NONE);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "BID");
  lua_pushnumber(&state, Side::BID);
  lua_settable(&state, -3);
  lua_pushliteral(&state, "ASK");
  lua_pushnumber(&state, Side::ASK);
  lua_settable(&state, -3);
  lua_setglobal(&state, "Side");
}

void Spire::ExportTaskState(lua_State& state) {
  lua_newtable(&state);
  lua_pushliteral(&state, "READY");
  lua_pushnumber(&state, static_cast<int>(Task::State::READY));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "INITIALIZING");
  lua_pushnumber(&state, static_cast<int>(Task::State::INITIALIZING));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "ACTIVE");
  lua_pushnumber(&state, static_cast<int>(Task::State::ACTIVE));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "PENDING_CANCEL");
  lua_pushnumber(&state, static_cast<int>(Task::State::PENDING_CANCEL));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "CANCELED");
  lua_pushnumber(&state, static_cast<int>(Task::State::CANCELED));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "FAILED");
  lua_pushnumber(&state, static_cast<int>(Task::State::FAILED));
  lua_settable(&state, -3);
  lua_pushliteral(&state, "COMPLETE");
  lua_pushnumber(&state, static_cast<int>(Task::State::COMPLETE));
  lua_settable(&state, -3);
  lua_setglobal(&state, "TaskState");
}

void Spire::ExportLuaTypes(lua_State& state) {
  ExportOrderStatus(state);
  ExportOrderType(state);
  ExportSide(state);
  ExportTaskState(state);
}
