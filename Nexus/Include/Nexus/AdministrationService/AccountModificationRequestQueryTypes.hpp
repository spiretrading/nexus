#ifndef NEXUS_ACCOUNT_MODIFICATION_REQUEST_QUERY_TYPES_HPP
#define NEXUS_ACCOUNT_MODIFICATION_REQUEST_QUERY_TYPES_HPP
#include <boost/mp11/list.hpp>
#include <Beam/Queries/EvaluatorTranslator.hpp>
#include "Nexus/AdministrationService/AccountModificationRequest.hpp"

namespace Nexus {

  /** Stores the types used to query AccountModificationRequests. */
  struct AccountModificationRequestQueryTypes {
    using ExtendedNativeTypes =
      boost::mp11::mp_list<AccountModificationRequest>;
    using NativeTypes = boost::mp11::mp_append<
      Beam::QueryTypes::NativeTypes, ExtendedNativeTypes>;
    using ValueTypes = Beam::QueryTypes::ValueTypes;
    using ComparableTypes = Beam::QueryTypes::ComparableTypes;
  };
}

#endif
