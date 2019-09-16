#ifndef SPIRE_LUA_REACTOR_HPP
#define SPIRE_LUA_REACTOR_HPP
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}
#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include <Beam/Collections/Enum.hpp>
#include <Beam/Utilities/FunctionObject.hpp>
#include "Spire/Canvas/LuaNodes/LuaReactorParameter.hpp"

namespace Spire {

  /**
   * Pops a value from Lua's evaluation stack.
   * @param <T> The type of the value to pop.
   */
  template<typename T>
  struct PopLuaValue {
    T operator ()(lua_State& state) const;
  };

  template<typename T>
  struct LuaReactorCore {
    using Type = T;
    std::string m_functionName;
    std::vector<std::unique_ptr<LuaReactorParameter>> m_parameters;
    lua_State* m_luaState;

    LuaReactorCore(std::string functionName,
      std::vector<std::unique_ptr<LuaReactorParameter>> parameters,
      lua_State& luaState)
      : m_functionName{std::move(functionName)},
        m_parameters{std::move(parameters)},
        m_luaState{&luaState} {}

    ~LuaReactorCore() {
      lua_close(m_luaState);
    }

    boost::optional<Type> operator ()(
        const std::vector<std::shared_ptr<BaseReactor>>& children) {
      lua_getglobal(m_luaState, m_functionName.c_str());
      auto parameterCount = 0;
      for(auto& parameter : m_parameters) {
        try {
          parameter->Push(*m_luaState);
        } catch(const std::exception& e) {
          lua_pop(m_luaState, parameterCount + 1);
          BOOST_THROW_EXCEPTION(ReactorError{e.what()});
        }
        ++parameterCount;
      }
      if(lua_pcall(m_luaState, children.size(), 1, 0) != 0) {
        BOOST_THROW_EXCEPTION(ReactorError{lua_tostring(m_luaState, -1)});
      } else {
        if(!lua_isnil(m_luaState, -1)) {
          try {
            return PopLuaValue<Type>{}(*m_luaState);
          } catch(const std::exception& e) {
            lua_pop(m_luaState, 1);
            BOOST_THROW_EXCEPTION(ReactorError{e.what()});
          }
        }
      }
      return boost::none;
    }
  };

  /**
   * Makes a LuaReactor.
   * @param functionName The name of the Lua function to call.
   * @param parameters The parameters to pass to the function.
   * @param luaState The Lua interpreter state used to invoke the function.
   */
  template<typename T>
  auto MakeLuaReactor(std::string functionName,
      std::vector<std::unique_ptr<LuaReactorParameter>> parameters,
      lua_State& luaState) {
    std::vector<std::shared_ptr<BaseReactor>> reactors(parameters.size());
    std::transform(parameters.begin(), parameters.end(),
      std::back_inserter(reactors),
      [] (auto& parameter) {
        return parameter->GetReactor();
      });
    auto core = MakeFunctionObject(std::make_unique<LuaReactorCore<T>>(
      std::move(functionName), std::move(parameters), luaState));
    auto reactor = MakeMultiReactor(std::move(core), std::move(reactors));
    return reactor;
  }

  template<>
  struct PopLuaValue<bool> {
    bool operator ()(lua_State& state) const {
      return static_cast<bool>(lua_toboolean(&state, -1));
    }
  };

  template<>
  struct PopLuaValue<boost::posix_time::ptime> {
    boost::posix_time::ptime operator ()(lua_State& state) const {

      // TODO
      return boost::posix_time::ptime{};
    }
  };

  template<>
  struct PopLuaValue<boost::posix_time::time_duration> {
    boost::posix_time::time_duration operator ()(lua_State& state) const {

      // TODO
      return boost::posix_time::time_duration{};
    }
  };

  template<>
  struct PopLuaValue<double> {
    double operator ()(lua_State& state) const {
      return static_cast<double>(lua_tonumber(&state, -1));
    }
  };

  template<typename T, std::size_t N>
  struct PopLuaValue<Beam::Enum<T, N>> {
    Beam::Enum<T, N> operator ()(lua_State& state) const {
      return static_cast<Beam::Enum<T, N>>(static_cast<int>(
        lua_tonumber(&state, -1)));
    }
  };

  template<std::size_t N>
  struct PopLuaValue<Beam::FixedString<N>> {
    Beam::FixedString<N> operator ()(lua_State& state) const {
      return lua_tostring(&state, -1);
    }
  };

  template<>
  struct PopLuaValue<int> {
    int operator ()(lua_State& state) const {
      return static_cast<int>(lua_tonumber(&state, -1));
    }
  };

  template<>
  struct PopLuaValue<std::int64_t> {
    std::int64_t operator ()(lua_State& state) const {
      return static_cast<std::int64_t>(lua_tonumber(&state, -1));
    }
  };

  template<>
  struct PopLuaValue<std::string> {
    std::string operator ()(lua_State& state) const {
      return lua_tostring(&state, -1);
    }
  };

  template<>
  struct PopLuaValue<std::uint16_t> {
    std::uint16_t operator ()(lua_State& state) const {
      return static_cast<std::uint16_t>(lua_tonumber(&state, -1));
    }
  };
}

#endif
