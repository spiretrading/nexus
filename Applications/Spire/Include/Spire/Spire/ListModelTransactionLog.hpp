#ifndef SPIRE_LIST_MODEL_TRANSACTION_LOG_HPP
#define SPIRE_LIST_MODEL_TRANSACTION_LOG_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/ModelTransactionLog.hpp"

namespace Spire {
  template<typename T>
  using ListModelTransactionLog = ModelTransactionLog<ListModel<T>>;
}

#endif
