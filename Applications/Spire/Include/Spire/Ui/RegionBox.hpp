#ifndef SPIRE_REGION_BOX_HPP
#define SPIRE_REGION_BOX_HPP
#include "Nexus/Definitions/Region.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Defines a model over a Region. */
  using RegionModel = ValueModel<Nexus::Region>;

  /** Defines a local model for a Region. */
  using LocalRegionModel = LocalValueModel<Nexus::Region>;

  /** Displays a TagComboBox over an open set of region values. */
  class RegionBox : public QWidget {
    public:
    
      /**
       * Signals the submission of the region.
       * @param submission The list of values to be submitted.
       */
      using SubmitSignal = Signal<void (const Nexus::Region& submission)>;

      /**
       * Constructs a RegionBox using a default current model.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      explicit RegionBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a RegionBox.
       * @param query_model The model used to query matches.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      RegionBox(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<RegionModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the current model. */
      const std::shared_ptr<RegionModel>& get_current() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this RegionBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the RegionBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    private:
      struct RegionQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<RegionQueryModel> m_query_model;
      std::shared_ptr<RegionModel> m_current;
      Nexus::Region m_last_region;
      TagComboBox* m_tag_combo_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_tag_operation_connection;

      void on_current(const Nexus::Region& region);
      void on_submit(const std::shared_ptr<AnyListModel>& submission);
      void on_tags_operation(const AnyListModel::Operation& operation);
  };
}

#endif
