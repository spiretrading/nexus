#ifndef SPIRE_TEST_KEY_SEQUENCE_MODEL_HPP
#define SPIRE_TEST_KEY_SEQUENCE_MODEL_HPP
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Spire/ValueModel.hpp"

namespace Spire {

  class TestKeySequenceModel : public KeySequenceModel {
    public:

      QValidator::State get_state() const override;

      const QKeySequence& get_current() const override;

      QValidator::State set_current(const QKeySequence& sequence) override;

      boost::signals2::connection connect_current_signal(
        const CurrentSignal::slot_type& slot) const override;

    private:
      mutable CurrentSignal m_current_signal;
      QKeySequence m_current;
  };
}

#endif
