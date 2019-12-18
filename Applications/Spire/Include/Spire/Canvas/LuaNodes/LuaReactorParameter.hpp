#ifndef SPIRE_LUA_REACTOR_PARAMETER_HPP
#define SPIRE_LUA_REACTOR_PARAMETER_HPP
extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}
#include <memory>
#include <Aspen/Aspen.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /** Base class for a parameter passed into a Lua script. */
  class LuaReactorParameter {
    public:
      using Type = void;

      virtual ~LuaReactorParameter() = default;

      /** Pushes the parameter onto the Lua stack */
      virtual void Push(lua_State& luaState) const = 0;

      virtual Aspen::State commit(int sequence) noexcept = 0;

      virtual void eval() const noexcept = 0;

    protected:
      LuaReactorParameter() = default;

    private:
      LuaReactorParameter(const LuaReactorParameter&) = delete;
      LuaReactorParameter(LuaReactorParameter&&) = delete;
  };
}

#endif
