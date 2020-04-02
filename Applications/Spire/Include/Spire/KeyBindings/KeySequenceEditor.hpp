#ifndef SPIRE_KEY_SEQUENCE_EDITOR_HPP
#define SPIRE_KEY_SEQUENCE_EDITOR_HPP
#include <set>
#include <QLineEdit>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class KeySequenceEditor : public QLineEdit {
    public:

      using FinishedSignal = Signal<void ()>;

      KeySequenceEditor(const QKeySequence& sequence,
        const std::set<Qt::Key>& valid_keys,
        QWidget* parent = nullptr);

      const QKeySequence& get_key_sequence() const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

    private:
      QKeySequence m_key_sequence;
      std::set<Qt::Key> m_valid_keys;
      std::vector<Qt::Key> m_entered_keys;

      bool is_valid(const std::vector<Qt::Key>& keys);
  };
}

#endif
