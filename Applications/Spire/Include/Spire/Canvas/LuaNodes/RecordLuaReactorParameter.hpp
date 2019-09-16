#ifndef SPIRE_RECORDLUAREACTORPARAMETER_HPP
#define SPIRE_RECORDLUAREACTORPARAMETER_HPP
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/LuaNodes/LuaReactorParameter.hpp"

namespace Spire {

  /** Implements a LuaReactorParameter for a Record. */
  class RecordLuaReactorParameter : public LuaReactorParameter {
    public:

      //! Constructs a RecordLuaReactorParameter.
      /*!
        \param reactor The Reactor representing the Record parameter.
        \param recordType The type of Record represented.
      */
      RecordLuaReactorParameter(Aspen::Box<Record>> reactor,
        const RecordType& recordType);

      void Push(lua_State& state) override const;

    private:
      std::shared_ptr<RecordType> m_recordType;
  };
}

#endif
