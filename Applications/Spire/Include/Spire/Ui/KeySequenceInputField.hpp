#ifndef SPIRE_KEY_SEQUENCE_INPUT_FIELD_HPP
#define SPIRE_KEY_SEQUENCE_INPUT_FIELD_HPP
#include <QKeySequence>
#include <QLineEdit>
#include <QPainter>
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Reprents an input for key sequences.
  class KeySequenceInputField : public QLineEdit {
    public:

      //! Constructs a KeySequenceInputField.
      /*!
        \param model The associated validation model.
        \param parent The parent widget.
      */
      KeySequenceInputField(std::shared_ptr<KeySequenceValidationModel> model,
        QWidget* parent = nullptr);

      //! Adds a key to the list of unvalidated input keys.
      /*
        \param key The key to add.
      */
      void add_key(Qt::Key key);

      //! Returns the last valid inputted key sequence.
      const QKeySequence& get_key_sequence() const;

      //! Sets the current key sequence iff it's a valid sequence.
      void set_key_sequence(const QKeySequence& sequence);

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      enum class State {
        DEFAULT,
        EDIT
      };

      std::shared_ptr<KeySequenceValidationModel> m_model;
      State m_state;
      QKeySequence m_key_sequence;
      std::vector<Qt::Key> m_entered_keys;
      QFont m_font;
      bool m_is_last_key_event_release;

      void commit_sequence(const QKeySequence& sequence);
      void draw_key(const QString& text, const QSize& text_size,
        const QPoint& pos, QPainter& painter) const;
  };
};

#endif
