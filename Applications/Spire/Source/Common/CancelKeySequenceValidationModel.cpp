#include "Spire/Spire/CancelKeySequenceValidationModel.hpp"

using namespace Spire;

CancelKeySequenceValidationModel::CancelKeySequenceValidationModel()
  : m_valid_sequences({}) {}

bool CancelKeySequenceValidationModel::is_valid(
    const QKeySequence& sequence) const {
  if(sequence.count() == 0) {
    return false;
  }
  for(auto& sequence : m_valid_sequences) {
    for(auto i = std::size_t(0); i < sequence.size(); ++i) {
      if(!sequence[i].contains(sequence[i])) {
        break;
      }
      if(i == sequence.size() - 1) {
        return true;
      }
    }
  }
  return false;
}
