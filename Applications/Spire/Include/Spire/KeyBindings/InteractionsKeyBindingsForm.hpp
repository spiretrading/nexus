#ifndef SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#define SPIRE_INTERACTIONS_KEY_BINDINGS_FORM_HPP
#include <array>
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/ProxyScalarValueModel.hpp"
#include "Spire/Spire/ProxyValueModel.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a form for the interactions key bindings of a region. */
  class InteractionsKeyBindingsForm : public QWidget {
    public:

      /**
       * Constructs an InteractionsKeyBindingsForm.
       * @param key_bindings The KeyBindingsModel storing all of the user's
       *        interactions.
       * @param region The region whose interactions are to be displayed.
       * @param parent The parent widget.
       */
      InteractionsKeyBindingsForm(
        std::shared_ptr<KeyBindingsModel> key_bindings,
        std::shared_ptr<RegionModel> region, QWidget* parent = nullptr);

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<RegionModel> m_region;
      TextAreaBox* m_description;
  };
}

#endif
