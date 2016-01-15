#ifndef NEXUS_ORDEREXECUTIONDATASTOREEXCEPTION_HPP
#define NEXUS_ORDEREXECUTIONDATASTOREEXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderExecutionDataStoreException
      \brief Exception to indicate an operation on an OrderExecutionDataStore
             failed.
   */
  class OrderExecutionDataStoreException : public Beam::IO::IOException {
    public:

      //! Constructs an OrderExecutionDataStoreException.
      OrderExecutionDataStoreException();

      //! Constructs an OrderExecutionDataStoreException.
      /*!
        \param message A message describing the error.
      */
      OrderExecutionDataStoreException(const std::string& message);

      virtual ~OrderExecutionDataStoreException() throw();
  };

  inline OrderExecutionDataStoreException::OrderExecutionDataStoreException()
      : Beam::IO::IOException{"Operation failed"} {}

  inline OrderExecutionDataStoreException::OrderExecutionDataStoreException(
      const std::string& message)
      : Beam::IO::IOException{message} {}

  inline OrderExecutionDataStoreException::~OrderExecutionDataStoreException()
      throw() {}
}
}

#endif
