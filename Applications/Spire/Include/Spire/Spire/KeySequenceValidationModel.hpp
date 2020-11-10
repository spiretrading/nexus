#ifndef SPIRE_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include <QKeySequence>

namespace Spire {

  //! Represents a model for validating key sequences.
  class KeySequenceValidationModel {
    public:

      virtual ~KeySequenceValidationModel() = default;
  
      //! Returns true if the given key sequence is valid.
      /*!
        \param sequence The key sequence to validate.
      */
      virtual bool is_valid(const QKeySequence& sequence) const = 0;
  };
}

#endif
