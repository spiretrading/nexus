#ifndef NEXUS_ADMINISTRATION_DATA_STORE_EXCEPTION_HPP
#define NEXUS_ADMINISTRATION_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>

namespace Nexus {

  /**
   * Exception to indicate an operation on an AdministrationDataStore failed.
   */
  class AdministrationDataStoreException : public Beam::IOException {
    public:
      using Beam::IOException::IOException;

      /** Constructs an AdministrationDataStoreException. */
      AdministrationDataStoreException();
  };

  inline AdministrationDataStoreException::AdministrationDataStoreException()
    : AdministrationDataStoreException("Operation failed") {}
}

#endif
