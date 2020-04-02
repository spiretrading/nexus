#ifndef SPIRE_KEY_SEQUENCE_EDITOR_HPP
#define SPIRE_KEY_SEQUENCE_EDITOR_HPP
#include <set>
#include <QLineEdit>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  class KeySequenceEditor : public QLineEdit {
    public:

      using KeySequenceSignal = Signal<void (const QKeySequence& keys)>;

      KeySequenceEditor(const std::set<Qt::Key>& valid_keys,
        QWidget* parent = nullptr);

      boost::signals2::connection connect_key_sequence_signal(
        const KeySequenceSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable KeySequenceSignal m_key_sequence_signal;
      std::set<Qt::Key> m_valid_keys;
      std::vector<Qt::Key> m_entered_keys;
      QFont m_font;

      bool is_valid(const std::vector<Qt::Key>& keys);
  };
}

#endif
