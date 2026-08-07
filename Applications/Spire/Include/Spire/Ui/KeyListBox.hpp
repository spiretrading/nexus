#ifndef SPIRE_KEY_LIST_BOX_HPP
#define SPIRE_KEY_LIST_BOX_HPP
#include <QKeySequence>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/KeyInputBox.hpp"
#include "Spire/Ui/TagBox.hpp"

namespace Spire {

  /** The type of model used for a list of QKeySequence. */
  using KeySequenceListModel = ListModel<QKeySequence>;

  /** Displays an AnyTagBox using KeyInputBox as the input box. */
  class KeyListBox : public AnyTagBox {
    public:

      /**
       * Constructs a KeyListBox.
       * @param keys The list model which holds a list of QKeySequence.
       * @param current The KeyInputBox's current model.
       * @param parent The parent widget.
       */
      KeyListBox(std::shared_ptr<KeySequenceListModel> keys,
        std::shared_ptr<KeySequenceValueModel> current,
        QWidget* parent = nullptr);

      /** Returns the list of key sequences. */
      std::shared_ptr<KeySequenceListModel> get_keys() const;

      /** Returns the current model. */
      std::shared_ptr<KeySequenceValueModel> get_current() const;

    protected:
      bool is_input_empty() const override;

    private:
      boost::signals2::scoped_connection m_key_submit_connection;
      boost::signals2::scoped_connection m_keys_connection;

      void on_submit(const AnyRef& submission);
      void on_keys_operation(const KeySequenceListModel::Operation& operation);
      void update_prompt_visibility();
  };
}

#endif
