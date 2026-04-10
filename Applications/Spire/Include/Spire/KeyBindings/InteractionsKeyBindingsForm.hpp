#ifndef SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#define SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#include <array>
#include <QWidget>
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Ui/ScopeDropDownBox.hpp"

namespace Spire {
  class TextAreaBox;

  /** Implements a form for the interactions key bindings of a scope. */
  class InteractionsKeyBindingsForm : public QWidget {
    public:

      /**
       * Constructs an InteractionsKeyBindingsForm.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param scope The scope whose interactions are to be displayed.
       * @param parent The parent widget.
       */
      InteractionsKeyBindingsForm(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<ScopeModel> scope, QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<ScopeModel> m_scope;
      TextAreaBox* m_description;
  };
}

#endif
