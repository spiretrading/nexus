#ifndef NEXUS_COMPLIANCERULEDATASTOREEXCEPTION_HPP
#define NEXUS_COMPLIANCERULEDATASTOREEXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ComplianceRuleDataStoreException
      \brief Exception to indicate an operation on a ComplianceRuleDataStore
             failed.
   */
  class ComplianceRuleDataStoreException : public Beam::IO::IOException {
    public:

      //! Constructs a ComplianceRuleDataStoreException.
      ComplianceRuleDataStoreException();

      //! Constructs a ComplianceRuleDataStoreException.
      /*!
        \param message A message describing the error.
      */
      ComplianceRuleDataStoreException(const std::string& message);

      virtual ~ComplianceRuleDataStoreException() throw();
  };

  inline ComplianceRuleDataStoreException::ComplianceRuleDataStoreException()
      : Beam::IO::IOException{"Operation failed."} {}

  inline ComplianceRuleDataStoreException::ComplianceRuleDataStoreException(
      const std::string& message)
      : Beam::IO::IOException{message} {}

  inline ComplianceRuleDataStoreException::~ComplianceRuleDataStoreException()
      throw() {}
}
}

#endif
