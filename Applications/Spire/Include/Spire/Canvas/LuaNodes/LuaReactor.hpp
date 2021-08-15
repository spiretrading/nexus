#ifndef SPIRE_LUA_REACTOR_HPP
#define SPIRE_LUA_REACTOR_HPP
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}
#include <cstdint>
#include <string>
#include <vector>
#include <Aspen/Aspen.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <Beam/Collections/Enum.hpp>
#include "Spire/Canvas/LuaNodes/LuaReactorParameter.hpp"

namespace Spire {

  /** Evaluates a reactor using a Lua defined function.
   *  @param <T> The type to evaluate to.
   */
  template<typename T>
  class LuaReactor {
    public:
      using Type = T;

      /**
       * Makes a LuaReactor.
       * @param functionName The name of the Lua function to call.
       * @param parameters The parameters to pass to the function.
       * @param luaState The Lua interpreter state used to invoke the function.
       */
      LuaReactor(std::string functionName,
        std::vector<Aspen::Unique<LuaReactorParameter>> parameters,
        lua_State& luaState);

      LuaReactor(LuaReactor&& reactor);

      ~LuaReactor();

      Aspen::State commit(int sequence) noexcept;

      Type eval() const noexcept;

      LuaReactor& operator =(LuaReactor&& reactor);

    private:
      std::string m_functionName;
      Aspen::CommitHandler<Aspen::Unique<LuaReactorParameter>> m_parameters;
      lua_State* m_luaState;
      Aspen::Maybe<Type> m_value;
  };

  /**
   * Pops a value from Lua's evaluation stack.
   * @param <T> The type of the value to pop.
   */
  template<typename T>
  struct PopLuaValue {
    T operator ()(lua_State& state) const;
  };

  template<typename T>
  LuaReactor<T>::LuaReactor(std::string functionName,
      std::vector<Aspen::Unique<LuaReactorParameter>> parameters,
      lua_State& luaState)
      : m_functionName(std::move(functionName)),
        m_parameters(std::move(parameters)),
        m_luaState(&luaState) {}

  template<typename T>
  LuaReactor<T>::LuaReactor(LuaReactor&& reactor)
    : m_functionName(std::move(reactor.m_functionName)),
      m_parameters(std::move(reactor.m_parameters)),
      m_luaState(std::move(reactor.m_luaState)),
      m_value(std::move(reactor.m_value)) {
    reactor.m_luaState = nullptr;
  }

  template<typename T>
  LuaReactor<T>::~LuaReactor() {
    if(m_luaState != nullptr) {
      lua_close(m_luaState);
    }
  }

  template<typename T>
  Aspen::State LuaReactor<T>::commit(int sequence) noexcept {
    auto state = m_parameters.commit(sequence);
    if(!Aspen::has_evaluation(state)) {
      return state;
    }
    lua_getglobal(m_luaState, m_functionName.c_str());
    auto parameterCount = 0;
    for(auto i = std::size_t(0); i != m_parameters.size(); ++i) {
      auto& parameter = m_parameters.get(i);
      try {
        parameter->Push(*m_luaState);
      } catch(...) {
        lua_pop(m_luaState, parameterCount + 1);
        m_value = std::current_exception();
        break;
      }
      ++parameterCount;
    }
    if(parameterCount != m_parameters.size()) {
      return state;
    }
    if(lua_pcall(m_luaState, parameterCount, 1, 0) != 0) {
      m_value = std::make_exception_ptr(std::runtime_error(
        lua_tostring(m_luaState, -1)));
    } else {
      if(!lua_isnil(m_luaState, -1)) {
        try {
          m_value = PopLuaValue<Type>()(*m_luaState);
        } catch(...) {
          lua_pop(m_luaState, 1);
          m_value = std::current_exception();
        }
      } else {
        state = Aspen::reset(state, Aspen::State::EVALUATED);
      }
    }
    return state;
  }

  template<typename T>
  typename LuaReactor<T>::Type LuaReactor<T>::eval() const noexcept {
    return m_value;
  }

  template<typename T>
  LuaReactor<T>& LuaReactor<T>::operator =(LuaReactor&& reactor) {
    if(m_luaState != nullptr) {
      lua_close(m_luaState);
    }
    m_functionName = std::move(reactor.m_functionName);
    m_parameters = std::move(reactor.m_parameters);
    m_luaState = std::move(reactor.m_luaState);
    m_value = std::move(reactor.m_value);
    reactor.m_luaState = nullptr;
    return *this;
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
