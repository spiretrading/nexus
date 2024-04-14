#ifndef SPIRE_CANCEL_KEY_BINDINGS_FORM_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_FORM_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  /** Implements a form for the cancel key bindings. */
  class CancelKeyBindingsForm : public QWidget {
    public:

      /**
       * Constructs a CancelKeyBindingsForm.
       * @param bindings The cancel key bindings to display.
       * @param parent The parent widget.
       */
      explicit CancelKeyBindingsForm(
        std::shared_ptr<CancelKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /** Returns the cancel key bindings. */
      const std::shared_ptr<CancelKeyBindingsModel>& get_bindings() const;
    
    private:
      std::shared_ptr<CancelKeyBindingsModel> m_bindings;
  };
}

#endif
