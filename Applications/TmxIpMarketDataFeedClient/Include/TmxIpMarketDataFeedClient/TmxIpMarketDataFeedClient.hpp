#ifndef TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#define TMX_IP_MARKET_DATA_FEED_CLIENT_HPP
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/Threading/Sync.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpClient.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpConfiguration.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpMessages.hpp"

namespace Nexus {

  /**
   * Receives and parses TMX IP messages without publishing market data.
   * @tparam C The client delivering ordered STAMP messages.
   */
  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  class TmxIpMarketDataFeedClient {
    public:

      /** The client delivering ordered STAMP messages. */
      using Client = Beam::dereference_t<C>;

      /**
       * Constructs a TMX IP market data feed client.
       * @param config The feed's configuration.
       * @param client Initializes the ordered message source.
       */
      template<Beam::Initializes<C> CF>
      TmxIpMarketDataFeedClient(TmxIpConfiguration config, CF&& client);

      ~TmxIpMarketDataFeedClient();

      /** Returns whether message reception has finished. */
      bool is_finished() const;

      /** Returns the exception that stopped message reception, if any. */
      std::exception_ptr get_exception() const;

      /** Stops message reception and closes the source client. */
      void close();

    private:
      TmxIpConfiguration m_config;
      Beam::local_ptr_t<C> m_client;
      Beam::Sync<std::exception_ptr> m_exception;
      std::atomic_bool m_is_finished;
      Beam::RoutineHandler m_read_loop;
      Beam::OpenState m_open_state;

      TmxIpMarketDataFeedClient(const TmxIpMarketDataFeedClient&) = delete;
      TmxIpMarketDataFeedClient& operator =(
        const TmxIpMarketDataFeedClient&) = delete;
      static void log(const StampMessage& message);
      void read_loop();
  };

  template<typename CF>
  TmxIpMarketDataFeedClient(TmxIpConfiguration, CF&&) ->
    TmxIpMarketDataFeedClient<std::remove_cvref_t<CF>>;

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  template<Beam::Initializes<C> CF>
  TmxIpMarketDataFeedClient<C>::TmxIpMarketDataFeedClient(
      TmxIpConfiguration config, CF&& client)
      : m_config(std::move(config)),
        m_client(std::forward<CF>(client)),
        m_is_finished(false) {
    m_read_loop = Beam::spawn([&] { read_loop(); });
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  TmxIpMarketDataFeedClient<C>::~TmxIpMarketDataFeedClient() {
    close();
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  bool TmxIpMarketDataFeedClient<C>::is_finished() const {
    return m_is_finished;
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  std::exception_ptr TmxIpMarketDataFeedClient<C>::get_exception() const {
    return m_exception.load();
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  void TmxIpMarketDataFeedClient<C>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_client->close();
    m_read_loop.wait();
    m_open_state.close();
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  void TmxIpMarketDataFeedClient<C>::log(const StampMessage& message) {
    auto out = std::stringstream();
    auto write_section = [&] (
        std::string_view name, const StampMessage::Section& section) {
      out << " (" << name;
      for(auto& field : section) {
        out << " (" << field.m_identifier << ' ' << field.m_index << ' ' <<
          std::quoted(field.m_value) << ')';
      }
      out << ')';
    };
    out << "(stamp";
    write_section("control", message.m_control_header);
    write_section("business", message.m_business_content);
    out << ")\n";
    std::cout << out.str() << std::flush;
  }

  template<typename C> requires IsTmxIpClient<Beam::dereference_t<C>>
  void TmxIpMarketDataFeedClient<C>::read_loop() {
    try {
      while(m_open_state.is_open()) {
        auto message = m_client->read();
        if(!m_open_state.is_open()) {
          break;
        }
        if(m_config.m_is_logging_messages) {
          log(message);
        }
        validate(message);
      }
    } catch(const std::exception&) {
      if(m_open_state.is_open()) {
        m_exception = std::current_exception();
      }
    }
    m_is_finished = true;
  }
}

#endif
