#ifndef NEXUS_TO_PYTHON_STAMP_MESSAGE_HPP
#define NEXUS_TO_PYTHON_STAMP_MESSAGE_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include "Nexus/Stamp/StampMessage.hpp"

namespace Nexus {

  /** Owns a STAMP message for use with Python. */
  class ToPythonStampMessage {
    public:

      /**
       * Parses an owned STAMP message.
       * @param source The message, excluding the transport framing and header.
       */
      explicit ToPythonStampMessage(std::string_view source);

      /** Returns the underlying message. */
      const StampMessage& get() const;

    private:
      Beam::SharedBuffer m_source;
      StampMessage m_message;
  };

  inline ToPythonStampMessage::ToPythonStampMessage(std::string_view source)
    : m_source(source.data(), source.size()),
      m_message(StampMessage::parse(
        std::string_view(m_source.get_data(), m_source.get_size()))) {}

  inline const StampMessage& ToPythonStampMessage::get() const {
    return m_message;
  }
}

#endif
