#ifndef SPIRE_KEY_SEQUENCE_EDITOR_HPP
#define SPIRE_KEY_SEQUENCE_EDITOR_HPP
#include <set>
#include <QLineEdit>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class KeySequenceEditor : public QLineEdit {
    public:

      //! Represents a valid key sequence, where the nth set in the list
      //! represents the nth key of a valid sequence.
      using ValidKeySequence = std::vector<std::set<Qt::Key>>;

      KeySequenceEditor(const QKeySequence& sequence,
        const std::vector<ValidKeySequence>& valid_sequences,
        QWidget* parent = nullptr);

      void add_key(Qt::Key key);

      const QKeySequence& get_key_sequence() const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QKeySequence m_key_sequence;
      std::vector<ValidKeySequence> m_valid_sequences;
      std::vector<Qt::Key> m_entered_keys;
      QFont m_font;

      void commit_sequence();
      bool is_valid(const std::vector<Qt::Key>& keys);
  };
}

#endif
