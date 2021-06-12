#include "Spire/Ui/TestKeySequenceModel.hpp"

using namespace boost::signals2;
using namespace Spire;

const QKeySequence& TestKeySequenceModel::get_current() const {
  return m_current;
}

QValidator::State TestKeySequenceModel::set_current(
    const QKeySequence& sequence) {
  for(auto i = 0; i < sequence.count(); ++i) {
    if((sequence[i] >= Qt::Key_0 && sequence[i] <= Qt::Key_9) ||
        sequence[i] >= Qt::Key_Exclam && sequence[i] <= Qt::Key_ParenRight) {
      return QValidator::Invalid;
    }
  }
  m_current = sequence;
  m_current_signal(m_current);
  return QValidator::Acceptable;
}

connection TestKeySequenceModel::connect_current_signal(
    const typename CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}
