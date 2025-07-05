#ifndef NEXUS_ADMINISTRATION_SERVICE_MESSAGE_HPP
#define NEXUS_ADMINISTRATION_SERVICE_MESSAGE_HPP
#include <ostream>
#include <string>
#include <vector>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include <Beam/ServiceLocator/DirectoryEntry.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Nexus::AdministrationService {

  /** Stores the details of a single message. */
  class Message {
    public:

      /** The type used to uniquely identify a message. */
      using Id = int;

      /** Stores the body of a message. */
      struct Body {

        /** The MIME-type of the message. */
        std::string m_content_type;

        /** The contents of the message. */
        std::string m_message;

        /** Returns an empty body. */
        static const Body EMPTY;

        /**
         * Makes a plain text body.
         * @param message The plain text message.
         */
        static Body make_plain_text(std::string message);

        bool operator ==(const Body&) const = default;
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
      Id get_id() const;

      /** Returns the account that sent the message. */
      const Beam::ServiceLocator::DirectoryEntry& get_account() const;

      /** Returns the timestamp when the message was received. */
      boost::posix_time::ptime get_timestamp() const;

      /** Returns the first body. */
      const Body& get_body() const;

      /** Returns the list of message bodies. */
      const std::vector<Body>& get_bodies() const;

      bool operator ==(const Message&) const = default;

    private:
      friend struct Beam::Serialization::Shuttle<Message>;
      Id m_id;
      Beam::ServiceLocator::DirectoryEntry m_account;
      boost::posix_time::ptime m_timestamp;
      std::vector<Body> m_bodies;
  };

  inline const auto Message::Body::EMPTY = Message::Body::make_plain_text("");

  inline std::ostream& operator <<(
      std::ostream& out, const Message::Body& body) {
    out << '(' << body.m_content_type << ' ' << body.m_message << ')';
    return out;
  }

  inline Message::Body Message::Body::make_plain_text(std::string message) {
    return Body("text/plain", std::move(message));
  }

  inline Message::Message()
      : m_id(-1) {
    m_bodies.reserve(1);
    m_bodies.push_back(Body::EMPTY);
  }

  inline Message::Message(Id id, Beam::ServiceLocator::DirectoryEntry account,
      boost::posix_time::ptime timestamp, std::vector<Body> bodies)
      : m_id(id),
        m_account(std::move(account)),
        m_timestamp(timestamp) {
    if(bodies.empty()) {
      m_bodies.reserve(1);
      m_bodies.push_back(Body::EMPTY);
    } else {
      m_bodies = std::move(bodies);
    }
  }

  inline Message::Id Message::get_id() const {
    return m_id;
  }

  inline const Beam::ServiceLocator::DirectoryEntry&
      Message::get_account() const {
    return m_account;
  }

  inline boost::posix_time::ptime Message::get_timestamp() const {
    return m_timestamp;
  }

  inline const Message::Body& Message::get_body() const {
    return m_bodies.front();
  }

  inline const std::vector<Message::Body>& Message::get_bodies() const {
    return m_bodies;
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<Nexus::AdministrationService::Message::Body> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::Message::Body& value,
        unsigned int version) const {
      shuttle.Shuttle("content_type", value.m_content_type);
      shuttle.Shuttle("message", value.m_message);
    }
  };

  template<>
  struct Shuttle<Nexus::AdministrationService::Message> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        Nexus::AdministrationService::Message& value,
        unsigned int version) const {
      shuttle.Shuttle("id", value.m_id);
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("timestamp", value.m_timestamp);
      shuttle.Shuttle("bodies", value.m_bodies);
      if(Beam::Serialization::IsReceiver<Shuttler>::value) {
        if(value.m_bodies.empty()) {
          value.m_bodies.push_back(
            Nexus::AdministrationService::Message::Body::EMPTY);
        }
      }
    }
  };
}

#endif
