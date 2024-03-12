#ifndef SPIRE_INTERACTIONS_PAGE_HPP
#define SPIRE_INTERACTIONS_PAGE_HPP
#include <QWidget>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Market.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/KeyBindingsModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a widget for the interactions key bindings. */
  class InteractionsPage : public QWidget {
    public:

      /**
       * Constructs an InteractionsPage.
       * @param key_bindings The KeyBindingsModel storing all user interactions.
       * @param parent The parent widget.
       */
      InteractionsPage(std::shared_ptr<KeyBindingsModel> key_bindings,
        const Nexus::CountryDatabase& countries,
        const Nexus::MarketDatabase& markets, QWidget* parent = nullptr);

      /** Returns the key bindings being displayed. */
      const std::shared_ptr<KeyBindingsModel>& get_key_bindings() const;

    private:
      std::shared_ptr<KeyBindingsModel> m_key_bindings;
      std::shared_ptr<ArrayListModel<Nexus::Region>> m_regions;
      std::shared_ptr<RegionModel> m_current_region;
      InteractionsKeyBindingsForm* m_interactions_form;
      ListView* m_list_view;
      AddRegionForm* m_add_region_form;

      QWidget* make_region_list_item(
        const std::shared_ptr<RegionListModel>& list, int index);
      void on_add_region_click();
      void on_current_index(const boost::optional<int>& current);
      void on_add_region(const Nexus::Region& region);
      void on_delete_region(const Nexus::Region& region);
  };
}

#endif
