#ifndef SPIRE_TASK_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_TASK_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include "Spire/Spire/KeySequenceValidationModel.hpp"

namespace Spire {

  class TaskKeySequenceValidationModel : public KeySequenceValidationModel {
    public:

      //! Constructs a TaskKeySequenceValidationModel.
      TaskKeySequenceValidationModel();

      virtual ~TaskKeySequenceValidationModel() = default;

      bool is_valid(const QKeySequence& sequence) const override;

    private:
      std::vector<ValidKeySequences> m_valid_sequences;
  };
}

#endif
