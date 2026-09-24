#ifndef OTC_LINK_RECOVERY_CLIENT_HPP
#define OTC_LINK_RECOVERY_CLIENT_HPP
#include <cstring>
#include <functional>
#include <memory>
#include <stop_token>
#include <vector>
#include <Beam/IO/Channel.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Routines/Async.hpp>
#include <Beam/Threading/Mutex.hpp>
#include <Beam/Threading/Sync.hpp>
#include <Beam/TimeService/Timer.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryMessages.hpp"

namespace Nexus {

  /**
   * Recovers message ranges from one OTC Link channel.
   * @tparam C The channel carrying requests and recovered messages.
   * @tparam T The timer bounding each request.
   */
  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  class OtcLinkRecoveryClient {
    public:

      /** The channel carrying requests and recovered messages. */
      using Channel = C;

      /** The timer bounding each request. */
      using Timer = Beam::dereference_t<T>;

      /**
       * Constructs an OtcLinkRecoveryClient.
       * @param sender The subscriber's SenderCompID.
       * @param channel The real-time channel's RefApplID.
       * @param connection_builder Opens a connection for each request,
       *        honoring cancellation.
       * @param timer The timer bounding connection, writing and reading.
       */
      template<Beam::Initializes<T> TF>
      OtcLinkRecoveryClient(std::string sender, std::uint16_t channel,
        std::function<std::shared_ptr<C> (std::stop_token)> connection_builder,
        TF&& timer);

      ~OtcLinkRecoveryClient();

      /**
       * Requests a range once and returns its complete encoded messages.
       * Throws if the range cannot be fully recovered.
       * @param sequence The first missing message sequence.
       * @param count The number of messages, at most
       *        OtcLinkRecoveryRequest::MAXIMUM_COUNT.
       * @param stop_token Cancels this request.
       */
      std::vector<Beam::SharedBuffer> request(std::uint32_t sequence,
        std::uint32_t count, std::stop_token stop_token);

      /** Requests a multicast snapshot and waits for its acknowledgement. */
      void request_snapshot(std::stop_token stop_token);

      /** Cancels the active request and closes the client. */
      void close();

    private:
      static constexpr auto BUFFER_SIZE = std::size_t(65536);
      struct Operation {
        struct State {
          std::shared_ptr<Channel> m_channel;
          std::exception_ptr m_exception;
        };
        Beam::Sync<State> m_state;
        std::stop_source m_stop_source;
      };
      std::string m_sender;
      std::uint16_t m_channel;
      std::function<std::shared_ptr<Channel> (std::stop_token)>
        m_connection_builder;
      Beam::local_ptr_t<T> m_timer;
      Beam::Mutex m_mutex;
      Beam::Sync<std::shared_ptr<Operation>> m_operation;
      std::uint64_t m_next_id;
      Beam::RoutineTaskQueue m_tasks;
      Beam::OpenState m_open_state;

      static void cancel(const std::shared_ptr<Operation>& operation);
      static void check(const Operation& operation);
      static void read_until(Channel& channel, Beam::SharedBuffer& buffer,
        std::string_view& payload, std::size_t size);
      static OtcLinkRecoveryResponse read_response(Channel& channel,
        Beam::SharedBuffer& buffer, std::string_view& payload);
      OtcLinkRecoveryClient(const OtcLinkRecoveryClient&) = delete;
      OtcLinkRecoveryClient& operator =(const OtcLinkRecoveryClient&) = delete;
      std::vector<Beam::SharedBuffer> request(
        OtcLinkRecoveryRequest request, std::stop_token stop_token);
      std::vector<Beam::SharedBuffer> execute(Operation& operation,
        const OtcLinkRecoveryRequest& request, const std::string& encoded);
      void finish(Operation& operation);
  };

  template<std::invocable<std::stop_token> F, typename T>
  OtcLinkRecoveryClient(std::string, std::uint16_t, F, T&&) ->
    OtcLinkRecoveryClient<
      Beam::dereference_t<std::invoke_result_t<F, std::stop_token>>,
      std::remove_cvref_t<T>>;

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  template<Beam::Initializes<T> TF>
  OtcLinkRecoveryClient<C, T>::OtcLinkRecoveryClient(std::string sender,
      std::uint16_t channel,
      std::function<std::shared_ptr<C> (std::stop_token)> connection_builder,
      TF&& timer)
      : m_sender(std::move(sender)),
        m_channel(channel),
        m_connection_builder(std::move(connection_builder)),
        m_timer(std::forward<TF>(timer)),
        m_next_id(1) {
    try {
      OtcLinkRecoveryRequest(m_sender, 0, m_channel, 1, 1).encode();
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkRecoveryClient<C, T>::~OtcLinkRecoveryClient() {
    close();
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  std::vector<Beam::SharedBuffer> OtcLinkRecoveryClient<C, T>::request(
      std::uint32_t sequence, std::uint32_t count, std::stop_token stop_token) {
    return request(OtcLinkRecoveryRequest(
      m_sender, 0, m_channel, sequence, count), stop_token);
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::request_snapshot(
      std::stop_token stop_token) {
    request(OtcLinkRecoveryRequest(m_sender, 0, m_channel, 0, 0,
      OtcLinkRecoveryRequest::Type::SNAPSHOT), stop_token);
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    if(auto operation = m_operation.load()) {
      cancel(operation);
    }
    auto lock = std::lock_guard(m_mutex);
    m_tasks.close();
    m_tasks.wait();
    m_open_state.close();
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::cancel(
      const std::shared_ptr<Operation>& operation) {
    auto channel = Beam::with(operation->m_state, [] (auto& state) {
      state.m_exception = std::make_exception_ptr(
        Beam::IOException("OTC Link recovery canceled or timed out."));
      return std::exchange(state.m_channel, {});
    });
    operation->m_stop_source.request_stop();
    if(channel) {
      channel->get_connection().close();
    }
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::check(const Operation& operation) {
    Beam::with(operation.m_state, [] (const auto& state) {
      if(state.m_exception) {
        std::rethrow_exception(state.m_exception);
      }
    });
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::read_until(Channel& channel,
      Beam::SharedBuffer& buffer, std::string_view& payload, std::size_t size) {
    if(payload.size() >= size) {
      return;
    }
    if(!payload.empty() && payload.data() != buffer.get_data()) {
      std::memmove(buffer.get_mutable_data(), payload.data(), payload.size());
    }
    buffer.shrink(buffer.get_size() - payload.size());
    while(buffer.get_size() < size) {
      channel.get_reader().read(
        Beam::out(buffer), BUFFER_SIZE - buffer.get_size());
    }
    payload = std::string_view(buffer.get_data(), buffer.get_size());
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  OtcLinkRecoveryResponse OtcLinkRecoveryClient<C, T>::read_response(
      Channel& channel, Beam::SharedBuffer& buffer, std::string_view& payload) {
    auto field = std::size_t(0);
    for(auto i = std::size_t(0); i < BUFFER_SIZE; ++i) {
      read_until(channel, buffer, payload, i + 1);
      if(payload[i] == '\x01') {
        if(payload.substr(field, i + 1 - field).starts_with("10=")) {
          auto response = OtcLinkRecoveryResponse::parse(
            payload.substr(0, i + 1));
          payload.remove_prefix(i + 1);
          return response;
        }
        field = i + 1;
      }
    }
    boost::throw_with_location(
      OtcLinkParserException("OTC Link acknowledgement is too long."));
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  std::vector<Beam::SharedBuffer> OtcLinkRecoveryClient<C, T>::request(
      OtcLinkRecoveryRequest request, std::stop_token stop_token) {
    auto lock = std::lock_guard(m_mutex);
    request.m_id = m_next_id++;
    auto encoded = request.encode();
    auto operation = std::make_shared<Operation>();
    Beam::with(m_operation, [&] (auto& current) {
      if(!m_open_state.is_open() || stop_token.stop_requested()) {
        boost::throw_with_location(Beam::EndOfFileException());
      }
      current = operation;
    });
    auto stop = std::stop_callback(stop_token, [=, this] {
      m_tasks.push([=] { cancel(operation); });
    });
    auto slot = Beam::callback<typename Timer::Result>(
      [=, this] (const auto& result) {
        if(result != Timer::Result::CANCELED) {
          m_tasks.push([=] { cancel(operation); });
        }
      });
    auto messages = std::vector<Beam::SharedBuffer>();
    try {
      m_timer->get_publisher().monitor(slot);
      m_timer->start();
      messages = execute(*operation, request, encoded);
    } catch(const std::exception&) {
      slot->close();
      finish(*operation);
      throw;
    }
    slot->close();
    finish(*operation);
    check(*operation);
    if(stop_token.stop_requested()) {
      boost::throw_with_location(Beam::EndOfFileException());
    }
    return messages;
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  std::vector<Beam::SharedBuffer> OtcLinkRecoveryClient<C, T>::execute(
      Operation& operation, const OtcLinkRecoveryRequest& request,
      const std::string& encoded) {
    check(operation);
    auto channel = m_connection_builder(operation.m_stop_source.get_token());
    auto is_installed = Beam::with(operation.m_state, [&] (auto& state) {
      if(state.m_exception) {
        return false;
      }
      state.m_channel = channel;
      return true;
    });
    if(!is_installed) {
      channel->get_connection().close();
      check(operation);
    }
    channel->get_writer().write(
      Beam::SharedBuffer(encoded.data(), encoded.size()));
    auto buffer = Beam::SharedBuffer();
    auto payload = std::string_view();
    auto response = read_response(*channel, buffer, payload);
    if(response.m_id != request.m_id ||
        response.m_channel != request.m_channel ||
        response.m_recipient != request.m_sender) {
      boost::throw_with_location(
        OtcLinkParserException("Mismatched OTC Link acknowledgement."));
    }
    if(response.m_status != OtcLinkRecoveryResponse::Status::ACCEPTED) {
      boost::throw_with_location(Beam::IOException(std::format(
        "OTC Link recovery rejected ({}): {}",
        static_cast<int>(response.m_status), response.m_text)));
    }
    if(request.m_type == OtcLinkRecoveryRequest::Type::SNAPSHOT) {
      return {};
    }
    if(response.m_first_sequence != request.m_sequence ||
        response.m_last_sequence != request.m_sequence + request.m_count - 1) {
      boost::throw_with_location(
        OtcLinkParserException("Mismatched OTC Link recovery range."));
    }
    auto messages = std::vector<Beam::SharedBuffer>();
    messages.reserve(request.m_count);
    for(auto i = std::uint32_t(0); i < request.m_count; ++i) {
      read_until(*channel, buffer, payload, sizeof(std::uint16_t));
      auto length = OtcLinkCursor(payload).read_uint16();
      if(length < OtcLinkMessage::HEADER_LENGTH + sizeof(std::uint32_t)) {
        boost::throw_with_location(
          OtcLinkParserException("Short OTC Link recovery message."));
      }
      read_until(*channel, buffer, payload, length);
      auto message = OtcLinkMessage::parse(payload.substr(0, length));
      if(message.get_cursor().read_uint32() != request.m_sequence + i) {
        boost::throw_with_location(
          OtcLinkParserException("Unexpected OTC Link recovery sequence."));
      }
      check(operation);
      messages.emplace_back(payload.data(), length);
      payload.remove_prefix(length);
    }
    return messages;
  }

  template<Beam::IsChannel C, typename T> requires
    Beam::IsTimer<Beam::dereference_t<T>>
  void OtcLinkRecoveryClient<C, T>::finish(Operation& operation) {
    m_timer->cancel();
    auto completion = Beam::Async<void>();
    m_tasks.push([&] { completion.get_eval().set(); });
    completion.get();
    auto channel = Beam::with(operation.m_state, [] (auto& state) {
      return std::exchange(state.m_channel, {});
    });
    if(channel) {
      channel->get_connection().close();
    }
    m_operation = {};
  }
}

#endif
