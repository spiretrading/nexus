#ifndef SPIRE_NATIVE_LUA_REACTOR_PARAMETER_HPP
#define SPIRE_NATIVE_LUA_REACTOR_PARAMETER_HPP
#include <cstdint>
#include <string>
#include <Aspen/Aspen.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Beam/Collections/Enum.hpp>
#include "Spire/Canvas/LuaNodes/LuaReactorParameter.hpp"

namespace Spire {

  /**
   * Function object used to push a value onto Lua's evaluation stack.
   * @param <T> The type of value to push onto Lua's evaluation stack.
   */
  template<typename T>
  struct PushLuaValue {
    void operator ()(lua_State& state, const T& value) const;
  };

  /**
   * Implements a LuaReactorParameter for a native type.
   * @param <T> The native type represented.
   */
  template<typename T>
  class NativeLuaReactorParameter final : public LuaReactorParameter {
    public:

      /** The native type represented. */
      using Type = T;

      /**
       * Constructs a NativeLuaReactorParameter.
       * @param reactor The Reactor representing the parameter.
       */
      explicit NativeLuaReactorParameter(Aspen::Box<Type> reactor);

      void Push(lua_State& luaState) const override;

    private:
      Aspen::Box<Type> m_reactor;
  };

  /**
   * Makes a NativeLuaReactorParameter.
   * @param parameter The Reactor representing the parameter.
   */
  template<typename Parameter>
  auto MakeNativeLuaReactorParameter(Parameter&& parameter) {
    return std::make_unique<
      NativeLuaReactorParameter<Aspen::reactor_result_t<
      Aspen::to_reactor_t<Parameter>>>(
      Aspen::Box(std::forward<Parameter>(parameter)));
  }

  template<typename T>
  NativeLuaReactorParameter<T>::NativeLuaReactorParameter(
    Aspen::Box<Type> reactor)
    : LuaReactorParameter(std::move(reactor)) {}

  template<typename T>
  void NativeLuaReactorParameter<T>::Push(lua_State& luaState) const {
    PushLuaValue<Type>()(luaState, m_reactor.eval());
  }

  template<>
  struct PushLuaValue<bool> {
    void operator ()(lua_State& state, bool value) const {
      lua_pushboolean(&state, value);
    }
  };

  template<>
  struct PushLuaValue<boost::posix_time::ptime> {
    void operator ()(lua_State& state,
        const boost::posix_time::ptime& value) const {

      // TODO
    }
  };

  template<>
  struct PushLuaValue<boost::posix_time::time_duration> {
    void operator ()(lua_State& state,
        const boost::posix_time::time_duration& value) const {

      // TODO
    }
  };

  template<>
  struct PushLuaValue<double> {
    void operator ()(lua_State& state, double value) const {
      lua_pushnumber(&state, value);
    }
  };

  template<typename T, std::size_t N>
  struct PushLuaValue<Beam::Enum<T, N>> {
    void operator ()(lua_State& state, Beam::Enum<T, N> value) const {
      lua_pushnumber(&state, static_cast<int>(value));
    }
  };

  template<std::size_t N>
  struct PushLuaValue<Beam::FixedString<N>> {
    void operator ()(lua_State& state,
        const Beam::FixedString<N>& value) const {
      lua_pushstring(&state, value.GetData());
    }
  };

  template<>
  struct PushLuaValue<int> {
    void operator ()(lua_State& state, int value) const {
      lua_pushnumber(&state, value);
    }
  };

  template<>
  struct PushLuaValue<std::int64_t> {
    void operator ()(lua_State& state, std::int64_t value) const {
      lua_pushnumber(&state, static_cast<int>(value));
    }
  };

  template<>
  struct PushLuaValue<std::string> {
    void operator ()(lua_State& state, const std::string& value) const {
      lua_pushstring(&state, value.c_str());
    }
  };

  template<>
  struct PushLuaValue<std::uint16_t> {
    void operator ()(lua_State& state, std::uint16_t value) const {
      lua_pushnumber(&state, value);
    }
  };
}

#endif
