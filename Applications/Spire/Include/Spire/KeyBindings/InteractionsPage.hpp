#ifndef SPIRE_INTERACTIONS_PAGE_HPP
#define SPIRE_INTERACTIONS_PAGE_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/ScopeDropDownBox.hpp"

namespace Spire {
  class AddScopeForm;
  class Button;
  class InteractionsKeyBindingsForm;
  class ScrollableListBox;

  /** Implements a widget for the interactions key bindings. */
  class InteractionsPage : public QWidget {
    public:

      /**
       * Constructs an InteractionsPage.
       * @param key_bindings The KeyBindingsModel storing all user interactions.
       * @param parent The parent widget.
       */
      explicit InteractionsPage(std::shared_ptr<KeyBindingsModel> key_bindings,
        QWidget* parent = nullptr);

      /** Returns the key bindings being displayed. */
      const std::shared_ptr<KeyBindingsModel>& get_key_bindings() const;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<ArrayListModel<Nexus::Scope>> m_scopes;
      std::shared_ptr<ListModel<Nexus::Scope>> m_available_scopes;
      std::shared_ptr<ScopeModel> m_current_scope;
      InteractionsKeyBindingsForm* m_interactions_form;
      ScrollableListBox* m_list_box;
      Button* m_add_scope_button;
      AddScopeForm* m_add_scope_form;

      QWidget* make_scope_list_item(
        const std::shared_ptr<ScopeListModel>& list, int index);
      void on_add_scope_click();
      void on_current_index(const boost::optional<int>& current);
      void on_add_scope(const Nexus::Scope& scope);
      void on_delete_scope(const Nexus::Scope& scope);
  };
}

#endif
