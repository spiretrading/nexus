#ifndef SPIRE_SECURITY_STACK_HPP
#define SPIRE_SECURITY_STACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include "Nexus/Definitions/Security.hpp"

namespace Spire {

  /** Stores a stack like cycle of security's viewed by a user. */
  class SecurityStack {
    public:

      /** Constructs an empty stack. */
      SecurityStack() = default;

      /** Pushes a security onto the stack. */
      void push(const Nexus::Security& security);

      /**
       * Pushes a security onto the stack and yields the top of the stack.
       * @param security The security to push.
       * @return The top of the stack.
       */
      Nexus::Security push_front(const Nexus::Security& security);

      /**
       * Pushes a security onto the stack and pops out the bottom of the stack.
       * @param security The security to push.
       * @return The bottom of the stack.
       */
      Nexus::Security push_back(const Nexus::Security& security);

    private:
      friend struct Beam::DataShuttle;
      std::deque<Nexus::Security> m_securities;

      template<Beam::IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<Beam::IsShuttle S>
  void SecurityStack::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("securities", m_securities);
  }
}

#endif
