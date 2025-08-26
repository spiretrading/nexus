#ifndef NEXUS_WEB_PORTAL_SESSION_HPP
#define NEXUS_WEB_PORTAL_SESSION_HPP
#include <mutex>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/WebServices/AuthenticatedSession.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Nexus/Clients/Clients.hpp"

namespace Nexus {

  /** Represents a session to the WebPortal. */
  class WebPortalSession : public Beam::WebServices::AuthenticatedSession {
    public:

      /**
       * Constructs a WebPortalSession.
       * @param id The session's id.
       */
      explicit WebPortalSession(std::string id);

      /** Returns the Clients used by this session. */
      Clients& get_clients();

      /**
       * Sets the Clients to use for this session.
       * @param clients The ServiceClients to use.
       */
      void set_clients(Clients clients);

      /**
       * Shuttles the parameters from this client.
       * @param request The request containing the parameters to deserialize.
       * @return The deserialized value.
       */
      template<typename T>
      auto shuttle_parameters(const Beam::WebServices::HttpRequest& request);

      /**
       * Shuttles the response to this client.
       * @param value The value to serialize.
       * @param response Stores the response.
       */
      template<typename T>
      void shuttle_response(
        const T& value, Beam::Out<Beam::WebServices::HttpResponse> response);

    private:
      mutable boost::mutex m_mutex;
      Beam::Serialization::JsonReceiver<Beam::IO::SharedBuffer> m_receiver;
      Beam::Serialization::JsonSender<Beam::IO::SharedBuffer> m_sender;
      boost::optional<Clients> m_clients;
  };

  template<typename T>
  auto WebPortalSession::shuttle_parameters(
      const Beam::WebServices::HttpRequest& request) {
    auto parameters = T();
    {
      auto lock = std::lock_guard(m_mutex);
      m_receiver.SetSource(Beam::Ref(request.GetBody()));
      m_receiver.Shuttle(parameters);
    }
    return parameters;
  }

  template<typename T>
  void WebPortalSession::shuttle_response(
      const T& value, Beam::Out<Beam::WebServices::HttpResponse> response) {
    response->SetHeader({"Content-Type", "application/json"});
    auto buffer = [&] {
      auto lock = std::lock_guard(m_mutex);
      return Beam::Serialization::Encode<Beam::IO::SharedBuffer>(
        m_sender, value);
    }();
    response->SetBody(std::move(buffer));
  }
}

#endif
