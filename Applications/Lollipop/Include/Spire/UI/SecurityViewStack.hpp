#ifndef SPIRE_SECURITYVIEWSTACK_HPP
#define SPIRE_SECURITYVIEWSTACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include <boost/optional/optional.hpp>
#include "Nexus/Definitions/Security.hpp"

namespace Spire {
namespace UI {

  /*! \class SecurityViewStack
      \brief Stores a stack like cycle of Security's viewed by a user.
   */
  class SecurityViewStack {
    public:

      //! Constructs a SecurityViewStack.
      SecurityViewStack();

      //! Pushes a Security onto the stack.
      void Push(const Nexus::Security& security);

      //! Pushes a Security onto the stack and yields the top of the stack.
      /*!
        \param security The Security to push.
        \param continuation The action to perform on the top of the stack.
      */
      void PushUp(const Nexus::Security& security,
        const std::function<void (const Nexus::Security&)>& continuation);

      //! Pushes a Security onto the stack and pops out the bottom of the stack.
      /*!
        \param security The Security to push.
        \param continuation The action to perform on the bottom of the stack.
      */
      void PushDown(const Nexus::Security& security,
        const std::function<void (const Nexus::Security&)>& continuation);

    private:
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Security> m_securities;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SecurityViewStack::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("securities", m_securities);
  }
}
}

#endif
