#ifndef SPIRE_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#define SPIRE_KEY_SEQUENCE_VALIDATION_MODEL_HPP
#include <vector>
#include <QKeySequence>
#include <QSet>

namespace Spire {

  //! Represents a model for validating key sequences.
  class KeySequenceValidationModel {
    public:

      //! Represents a collection of valid key sequences, where the nth set in
      //! the list represents the nth key of a potential valid sequence.
      using ValidKeySequences = std::vector<QSet<Qt::Key>>;
  
      //! Returns true if the given key sequence is valid.
      /*!
        \param sequence The key sequence to validate.
      */
      virtual bool is_valid(const QKeySequence& sequence) const = 0;
  };
}

#endif
