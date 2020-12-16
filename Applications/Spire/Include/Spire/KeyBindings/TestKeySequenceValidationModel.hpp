#ifndef SPIRE_TEST_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_TEST_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include "Spire/KeyBindings/KeySequenceValidationModel.hpp"

namespace Spire {

  //! Represents a model for testing a KeySequenceInputWidget.
  class TestKeySequenceValidationModel : public KeySequenceValidationModel {
    public:

      virtual ~TestKeySequenceValidationModel() = default;

      bool is_valid(const QKeySequence& sequence) const override;
  };
}

#endif
