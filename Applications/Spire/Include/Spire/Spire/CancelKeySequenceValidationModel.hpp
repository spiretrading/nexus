#ifndef SPIRE_CANCEL_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_CANCEL_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include "Spire/Spire/KeySequenceValidationModel.hpp"

namespace Spire {

  //! Represents a model for validating cancel key sequences.
  class CancelKeySequenceValidationModel : public KeySequenceValidationModel {
    public:

      //! Constructs a CancelKeySequenceValidationModel.
      CancelKeySequenceValidationModel();

      virtual ~CancelKeySequenceValidationModel() = default;

      bool is_valid(const QKeySequence& sequence) const override;

    private:
      std::vector<ValidKeySequences> m_valid_sequences;
  };
}

#endif
