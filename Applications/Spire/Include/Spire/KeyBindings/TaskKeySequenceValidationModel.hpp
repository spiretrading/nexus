#ifndef SPIRE_TASK_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_TASK_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include "Spire/KeyBindings/KeySequenceValidationModel.hpp"

namespace Spire {

  //! Represents a model for validing task key sequences.
  class TaskKeySequenceValidationModel : public KeySequenceValidationModel {
    public:

      bool is_valid(const QKeySequence& sequence) const override;
  };
}

#endif
