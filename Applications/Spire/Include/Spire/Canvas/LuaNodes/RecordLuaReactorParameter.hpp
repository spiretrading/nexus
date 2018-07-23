#ifndef SPIRE_RECORDLUAREACTORPARAMETER_HPP
#define SPIRE_RECORDLUAREACTORPARAMETER_HPP
#include <Beam/Reactors/LuaReactorParameter.hpp>
#include "Spire/Canvas/Canvas.hpp"

namespace Spire {

  /*! \class RecordLuaReactorParameter
      \brief Implements a LuaReactorParameter for a Record.
  */
  class RecordLuaReactorParameter : public Beam::Reactors::LuaReactorParameter {
    public:

      //! Constructs a RecordLuaReactorParameter.
      /*!
        \param reactor The Reactor representing the Record parameter.
        \param recordType The type of Record represented.
      */
      RecordLuaReactorParameter(
        std::shared_ptr<Beam::Reactors::Reactor<Record>> reactor,
        const RecordType& recordType);

      virtual void Push(lua_State& state) const;

    private:
      std::shared_ptr<RecordType> m_recordType;
  };
}

#endif
