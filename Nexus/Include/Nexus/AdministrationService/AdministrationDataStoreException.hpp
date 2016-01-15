#ifndef NEXUS_ADMINISTRATIONDATASTOREEXCEPTION_HPP
#define NEXUS_ADMINISTRATIONDATASTOREEXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/AdministrationService/AdministrationService.hpp"

namespace Nexus {
namespace AdministrationService {

  /*! \class AdministrationDataStoreException
      \brief Exception to indicate an operation on an AdministrationDataStore
             failed.
   */
  class AdministrationDataStoreException : public Beam::IO::IOException {
    public:

      //! Constructs an AdministrationDataStoreException.
      AdministrationDataStoreException();

      //! Constructs a AdministrationDataStoreException.
      /*!
        \param message A message describing the error.
      */
      AdministrationDataStoreException(const std::string& message);

      virtual ~AdministrationDataStoreException() throw();
  };

  inline AdministrationDataStoreException::AdministrationDataStoreException()
      : Beam::IO::IOException("Operation failed") {}

  inline AdministrationDataStoreException::AdministrationDataStoreException(
      const std::string& message)
      : Beam::IO::IOException(message) {}

  inline AdministrationDataStoreException::~AdministrationDataStoreException()
      throw() {}
}
}

#endif
