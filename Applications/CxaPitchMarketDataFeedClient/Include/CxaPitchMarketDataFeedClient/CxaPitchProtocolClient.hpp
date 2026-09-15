#ifndef CXA_PITCH_PROTOCOL_CLIENT_HPP
#define CXA_PITCH_PROTOCOL_CLIENT_HPP
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"

namespace Nexus {

  /** Concept satisfied by types delivering the blocks of a CXA PITCH feed. */
  template<typename T>
  concept IsCxaPitchProtocolClient = requires(T& t) {
    { t.read() } -> std::same_as<CxaPitchBlock>;
    { t.close() } -> std::same_as<void>;
  };

  /**
   * Reads the blocks delivered by a CXA PITCH multicast feed.
   * @tparam C The type of Channel delivering one complete datagram per read.
   */
  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  class CxaPitchProtocolClient {
    public:

      /** The type of channel receiving the feed. */
      using Channel = Beam::dereference_t<C>;

      /**
       * Constructs a CxaPitchProtocolClient.
       * @param channel The channel receiving the feed.
       */
      template<Beam::Initializes<C> CF>
      explicit CxaPitchProtocolClient(CF&& channel);

      ~CxaPitchProtocolClient();

      /**
       * Reads the next block from the feed.
       * The returned payload is valid until the next read or destruction.
       */
      CxaPitchBlock read();

      void close();

    private:
      Beam::local_ptr_t<C> m_channel;
      Beam::SharedBuffer m_buffer;
      Beam::OpenState m_open_state;

      CxaPitchProtocolClient(const CxaPitchProtocolClient&) = delete;
      CxaPitchProtocolClient& operator =(
        const CxaPitchProtocolClient&) = delete;
  };

  template<typename CF>
  CxaPitchProtocolClient(CF&&) ->
    CxaPitchProtocolClient<std::remove_cvref_t<CF>>;

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  CxaPitchProtocolClient<C>::CxaPitchProtocolClient(CF&& channel)
    try : m_channel(std::forward<CF>(channel)) {
    } catch(const std::exception&) {
      Beam::throw_nested_with_location(Beam::ConnectException(
        "Failed to initialize the CXA PITCH protocol client."));
    }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  CxaPitchProtocolClient<C>::~CxaPitchProtocolClient() {
    close();
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  CxaPitchBlock CxaPitchProtocolClient<C>::read() {
    Beam::try_or_nest([&] {
      reset(m_buffer);
      m_channel->get_reader().read(Beam::out(m_buffer));
    }, Beam::IOException("Failed to read CXA PITCH block."));
    return CxaPitchBlock::parse(
      std::string_view(m_buffer.get_data(), m_buffer.get_size()));
  }

  template<typename C> requires Beam::IsChannel<Beam::dereference_t<C>>
  void CxaPitchProtocolClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_channel->get_connection().close();
    m_open_state.close();
  }
}

#endif
