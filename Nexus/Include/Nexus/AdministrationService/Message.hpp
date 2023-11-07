#ifndef NEXUS_ADMINISTRATION_SERVICE_MESSAGE_HPP
#define NEXUS_ADMINISTRATION_SERVICE_MESSAGE_HPP
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus::AdministrationService {

  /** Stores the details of a single message. */
  class Message {
    public:

      /** The type used to uniquely identify a message. */
      using Id = int;

      /** Stores the body of a message. */
      struct Body {

        /** The MIME-type of the message. */
        std::string m_contentType;

        /** The contents of the message. */
        std::string m_message;

        /** Returns an empty body. */
        static const Body& EMPTY();

        /**
         * Makes a plain text body.
         * @param message The plaint text message.
         */
        static Body MakePlainText(std::string message);

        bool operator ==(const Body& rhs) const = default;
      };

      /** Constructs an empty Message. */
      Message();

      /**
       * Constructs a Message.
       * @param id The message's unique id.
       * @param account The account that sent the message.
       * @param timestamp The timestamp when the message was received.
       * @param bodies The list of message bodies.
       */
      Message(Id id, Beam::ServiceLocator::DirectoryEntry account,
        boost::posix_time::ptime timestamp, std::vector<Body> bodies);

      /** Returns this message's unique id. */
      Id GetId() const;

      /** Returns the account that sent the message. */
      const Beam::ServiceLocator::DirectoryEntry& GetAccount() const;

      /** Returns the timestamp when the message was received. */
      boost::posix_time::ptime GetTimestamp() const;

      /** Returns the first body. */
      const Body& GetBody() const;

      /** Returns the list of message bodies. */
      const std::vector<Body>& GetBodies() const;

    private:
      friend struct Beam::Serialization::Shuttle<Message>;
      Id m_id;
      Beam::ServiceLocator::DirectoryEntry m_account;
      boost::posix_time::ptime m_timestamp;
      std::vector<Body> m_bodies;
  };

  inline const Message::Body& Message::Body::EMPTY() {
    static auto value = MakePlainText({});
    return value;
  }

  inline Message::Body Message::Body::MakePlainText(std::string message) {
    return {"text/plain", std::move(message)};
  }

  inline Message::Message()
      : m_id(-1) {
    m_bodies.push_back(Body::EMPTY());
  }

  inline Message::Message(Id id, Beam::ServiceLocator::DirectoryEntry account,
      boost::posix_time::ptime timestamp, std::vector<Body> bodies)
      : m_id(id),
        m_account(std::move(account)),
        m_timestamp(timestamp) {
    if(bodies.empty()) {
      m_bodies.push_back(Body::EMPTY());
    } else {
      m_bodies = std::move(bodies);
    }
  }

  inline Message::Id Message::GetId() const {
    return m_id;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      Message::GetAccount() const {
    return m_account;
  }

  inline boost::posix_time::ptime Message::GetTimestamp() const {
    return m_timestamp;
  }

  inline const Message::Body& Message::GetBody() const {
    return m_bodies.front();
  }

  inline const std::vector<Message::Body>& Message::GetBodies() const {
    return m_bodies;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::Message::Body> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::Message::Body& value,
        unsigned int version) {
      shuttle.Shuttle("content_type", value.m_contentType);
      shuttle.Shuttle("message", value.m_message);
    }
  };

  template<>
  struct Shuttle<Nexus::AdministrationService::Message> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::Message& value, unsigned int version) {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("timestamp", value.m_timestamp);
      shuttle.Shuttle("bodies", value.m_bodies);
      if(Beam::Serialization::IsReceiver<Shuttler>::value) {
        if(value.m_bodies.empty()) {
          value.m_bodies.push_back(
            Nexus::AdministrationService::Message::Body::EMPTY());
        }
      }
    }
  };
}

#endif
