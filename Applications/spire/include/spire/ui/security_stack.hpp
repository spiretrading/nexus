#ifndef SPIRE_SECURITY_STACK_HPP
#define SPIRE_SECURITY_STACK_HPP
#include <deque>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDeque.hpp>
#include "Nexus/Definitions/Security.hpp"
#include "spire/ui/ui.hpp"

namespace spire {

  //! Stores a stack like cycle of security's viewed by a user.
  class SecurityStack {
    public:

      //! Constructs an empty stack.
      SecurityStack() = default;

      //! Pushes a security onto the stack.
      void push(Nexus::Security s);

      //! Pushes a security onto the stack and yields the top of the stack.
      /*!
        \param s The security to push.
        \return The top of the stack.
      */
      Nexus::Security push_front(Nexus::Security s);

      //! Pushes a security onto the stack and pops out the bottom of the stack.
      /*!
        \param s The security to push.
        \return The bottom of the stack.
      */
      Nexus::Security push_back(Nexus::Security s);

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
