#ifndef SPIRE_INTERACTIONS_PAGE_HPP
#define SPIRE_INTERACTIONS_PAGE_HPP
#include <QVBoxLayout>
#include <QWidget>
#include "Nexus/Definitions/Region.hpp"
#include "Nexus/Definitions/RegionMap.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Associates InteractionsKeyBindingsModel with a given Region. */
  using RegionInteractionsMap =
    Nexus::RegionMap<std::shared_ptr<InteractionsKeyBindingsModel>>;
  
  /** Implements a widget for the interactions key bindings. */
  class InteractionsPage : public QWidget {
    public:

      /** Signals that the region binding interactions has been added. */
      using AddSignal = Signal<void (const Nexus::Region& region,
        const std::shared_ptr<InteractionsKeyBindingsModel>& iteractions)>;

      /** Signals that the region binding interactions has been deleted. */
      using DeleteSignal = Signal<void (const Nexus::Region& region)>;

      /**
       * Constructs an InteractionsPage.
       * @param regions A list of regions for the user setting interactions up.
       * @param region_interactions The list of regions that the user has
       *        set up for interactions.
       * @param parent The parent widget.
       */
      InteractionsPage(std::shared_ptr<RegionListModel> regions,
        std::shared_ptr<RegionInteractionsMap> region_interactions,
        QWidget* parent = nullptr);

      /** Returns a list of regions for the user setting interactions up. */
      const std::shared_ptr<RegionListModel>& get_regions() const;

      /** Return a list of regions that the user has set up for interactions. */
      const std::shared_ptr<RegionInteractionsMap>&
        get_region_interactions() const;

     /** Connects a slot to AddSignal. */
      boost::signals2::connection connect_add_signal(
        const AddSignal::slot_type& slot) const;

     /** Connects a slot to DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    private:
      struct RegionInteractionsListModel;
      mutable AddSignal m_add_signal;
      mutable DeleteSignal m_delete_signal;
      std::shared_ptr<RegionListModel> m_regions;
      std::shared_ptr<RegionInteractionsListModel> m_list_model;
      std::shared_ptr<LocalRegionModel> m_list_view_current;
      ListView* m_list_view;
      QVBoxLayout* m_center_layout;
      AddRegionForm* m_add_region_form;
      Nexus::Region m_current_region;

      QWidget* make_interactions_form(const Nexus::Region& region) const;
      QWidget* make_list_item(
        const std::shared_ptr<RegionListModel>& list, int index);
      void on_add_region_click();
      void on_current_index(const boost::optional<int>& current);
      void on_current_region(const Nexus::Region& region);
      void on_add_region(const Nexus::Region& region);
      void on_delete_region(const Nexus::Region& region);
  };
}

#endif
