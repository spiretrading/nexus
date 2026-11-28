#ifndef SPIRE_ITEM_FILTER_HPP
#define SPIRE_ITEM_FILTER_HPP
#include <QWidget>
#include "Spire/Spire/AnyRef.hpp"

namespace Spire {

  /** Filters individual values and provides controls for its criteria. */
  class ItemFilter {
    public:

      /** Signals a change in filtering criteria. */
      using FilterSignal = Signal<void ()>;

      virtual ~ItemFilter() = default;

      /** Returns whether filtering criteria are active. */
      virtual bool is_active() const = 0;

      /**
       * Returns whether a value is excluded.
       * @param value The value to test.
       */
      virtual bool is_filtered(const AnyRef& value) const = 0;

      /**
       * Creates controls for editing the filtering criteria.
       * @param parent The parent widget.
       */
      virtual QWidget* make_widget(QWidget& parent) = 0;

      /** Clears the filtering criteria. */
      virtual void reset() = 0;

      /** Connects a slot to changes in filtering criteria. */
      virtual boost::signals2::connection connect_filter_signal(
        const FilterSignal::slot_type& slot) const = 0;

    protected:

      /** Constructs an ItemFilter. */
      ItemFilter() = default;

    private:
      ItemFilter(const ItemFilter&) = delete;
      ItemFilter& operator =(const ItemFilter&) = delete;
  };
}

#endif
