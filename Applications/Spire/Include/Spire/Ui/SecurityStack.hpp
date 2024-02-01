#ifndef SPIRE_SECURITY_STACK_HPP
#define SPIRE_SECURITY_STACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Ui/Ui.hpp"

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
      friend struct Beam::Serialization::DataShuttle;
      std::deque<Nexus::Security> m_securities;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  template<typename Shuttler>
  void SecurityStack::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("securities", m_securities);
  }
}

#endif
