#ifndef SPIRE_REGION_BOX_HPP
#define SPIRE_REGION_BOX_HPP
#include <QWidget>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Spire/QueryModel.hpp"

namespace Spire {
  template<typename> class TagComboBox;

  /** Defines a model over a Region. */
  using RegionModel = ValueModel<Nexus::Region>;

  /** Defines a local model for a Region. */
  using LocalRegionModel = LocalValueModel<Nexus::Region>;

  /** The type used to query for Regions. */
  using RegionQueryModel = QueryModel<Nexus::Region>;

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
       * @param regions The model used to region queries.
       * @param parent The parent widget.
       */
      explicit RegionBox(
        std::shared_ptr<RegionQueryModel> regions, QWidget* parent = nullptr);

      /**
       * Constructs a RegionBox.
       * @param regions The model used to region queries.
       * @param current The current value's model.
       * @param parent The parent widget.
       */
      RegionBox(std::shared_ptr<RegionQueryModel> regions,
        std::shared_ptr<RegionModel> current, QWidget* parent = nullptr);

      /** Returns the model used to query regions. */
      const std::shared_ptr<RegionQueryModel>& get_regions() const;

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
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<RegionQueryModel> m_regions;
      std::shared_ptr<RegionModel> m_current;
      Nexus::Region m_last_region;
      TagComboBox<Nexus::Region>* m_tag_combo_box;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_tag_operation_connection;

      void on_current(const Nexus::Region& region);
      void on_submit(
        const std::shared_ptr<ListModel<Nexus::Region>>& submission);
      void on_tags_operation(const AnyListModel::Operation& operation);
  };
}

#endif
