#ifndef OASIS_IRROC_LEGAL_ENTITY_IDENTIFIER_HPP
#define OASIS_IRROC_LEGAL_ENTITY_IDENTIFIER_HPP
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <boost/optional/optional.hpp>
#include <quickfix/Dictionary.h>
#include <quickfix/fix42/NewOrderSingle.h>

namespace Nexus {

  /** Encrypts and stores the IIROC LEI for participating members. */
  class IirocLegalEntityIdentifier {
    public:

      /**
       * Constructs an IirocLegalEntityIdentifier and encrypts the LEI provided
       * by a FIX session's config.
       * @param config The FIX session's config used to extract the LEI fields.
       */
      explicit IirocLegalEntityIdentifier(const FIX::Dictionary& config);

      /**
       * Populates a FIX new order single message with LEI fields.
       * @param new_order_single The FIX message to populate, no-op if LEI fields
       *        were not provided.
       */
      void populate(Beam::Out<FIX42::NewOrderSingle> new_order_single) const;

    private:
      boost::optional<std::string> m_broker_number;
      boost::optional<std::string> m_order_origination;
      boost::optional<std::string> m_lei;
  };
}

#endif
