#ifndef SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#define SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#include <QWidget>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a form for the interactions key bindings of a region. */
  class InteractionsKeyBindingsForm : public QWidget {
    public:

      /**
       * Constructs an InteractionsKeyBindingsForm.
       * @param region The region that the user has set up an interaction for.
       * @param bindings The interactions key bindings to display.
       * @param parent The parent widget.
       */
      explicit InteractionsKeyBindingsForm(Nexus::Region region,
        std::shared_ptr<InteractionsKeyBindingsModel> bindings,
        QWidget* parent = nullptr);

      /**
       * Sets the region to the InteractionsKeyBindingsForm.
       * @param region The region that the user has set up an interaction for.
       */
      void set_region(const Nexus::Region& region);

      /** Returns the interactions key bindings. */
      const std::shared_ptr<InteractionsKeyBindingsModel>& get_bindings() const;

    private:
      std::shared_ptr<InteractionsKeyBindingsModel> m_bindings;
      TextBox* m_header;
      TextAreaBox* m_description;
  };
}

#endif
