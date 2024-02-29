#ifndef SPIRE_ADD_REGION_FORM_HPP
#define SPIRE_ADD_REGION_FORM_HPP
#include <QWidget>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/RegionDropDownBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a panel to add a region. */
  class AddRegionForm : public QWidget {
    public:
    
      /**
       * Signals the submission of the region.
       * @param submission The region to be submitted.
       */
      using SubmitSignal = Signal<void (const Nexus::Region& submission)>;

       /**
       * Constructs an AddRegionForm.
       * @param regions A list of regions for the user to select.
       * @param parent The parent widget.
       */
      AddRegionForm(std::shared_ptr<RegionListModel> regions, QWidget& parent);

      /** Returns A list of regions for the user to select. */
      const std::shared_ptr<RegionListModel>& get_regions() const;
    
      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ListModel<Nexus::Region>> m_regions;
      RegionDropDownBox* m_region_drop_down_box;
      OverlayPanel* m_panel;

      void on_cancel();
      void on_add();
  };
}

#endif
