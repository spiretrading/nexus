#ifndef NEXUS_CLIENTWEBPORTALSESSION_HPP
#define NEXUS_CLIENTWEBPORTALSESSION_HPP
#include <boost/thread/mutex.hpp>
#include <Beam/Serialization/JsonReceiver.hpp>
#include <Beam/Serialization/JsonSender.hpp>
#include <Beam/WebServices/AuthenticatedSession.hpp>
#include <Beam/WebServices/HttpRequest.hpp>
#include <Beam/WebServices/HttpResponse.hpp>
#include "web_portal/web_portal/web_portal.hpp"

namespace Nexus {
namespace ClientWebPortal {

  /*! \class ClientWebPortalSession
      \brief Represents a session to the ClientWebPortal.
   */
  class ClientWebPortalSession :
      public Beam::WebServices::AuthenticatedSession {
    public:

      //! Constructs a ClientWebPortalSession.
      /*!
        \param id The session's id.
      */
      ClientWebPortalSession(std::string id);

      //! Shuttles the parameters from this client.
      /*!
        \param request The request containing the parameters to deserialize.
        \return The deserialized value.
      */
      template<typename T>
      T ShuttleParameters(const Beam::WebServices::HttpRequest& request);

      //! Shuttles the response to this client.
      /*!
        \param value The value to serialize.
        \param response Stores the response.
      */
      template<typename T>
      void ShuttleResponse(const T& value,
        Beam::Out<Beam::WebServices::HttpResponse> response);

    private:
      mutable boost::mutex m_mutex;
      Beam::Serialization::JsonReceiver<Beam::IO::SharedBuffer> m_receiver;
      Beam::Serialization::JsonSender<Beam::IO::SharedBuffer> m_sender;
  };

  template<typename T>
  T ClientWebPortalSession::ShuttleParameters(
      const Beam::WebServices::HttpRequest& request) {
    T parameters;
    {
      boost::lock_guard<boost::mutex> lock{m_mutex};
      m_receiver.SetSource(Beam::Ref(request.GetBody()));
      m_receiver.Shuttle(parameters);
    }
    return parameters;
  }

  template<typename T>
  void ClientWebPortalSession::ShuttleResponse(const T& value,
      Beam::Out<Beam::WebServices::HttpResponse> response) {
    response->SetHeader({"Content-Type", "application/json"});
    auto buffer =
      [&] {
        boost::lock_guard<boost::mutex> lock{m_mutex};
        return Beam::Serialization::Encode<Beam::IO::SharedBuffer>(m_sender,
          value);
      }();
    response->SetBody(std::move(buffer));
  }
}
}

#endif
