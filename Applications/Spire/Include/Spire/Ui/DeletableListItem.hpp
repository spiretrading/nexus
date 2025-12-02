#ifndef SPIRE_DELETABLE_LIST_ITEM_HPP
#define SPIRE_DELETABLE_LIST_ITEM_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"

namespace Spire {
  
  /** Represents a deletable item in a list. */
  class DeletableListItem : public QWidget {
    public:

      /** Signals that the item was being requested to delete. */
      using DeleteSignal = Signal<void ()>;

      /**
       * Constructs a DeletableListItem.
       * @param body The body of the item.
       * @param parent The parent widget.
       */
      explicit DeletableListItem(QWidget& body, QWidget* parent = nullptr);

      /** Returns the body of this item. */
      QWidget& get_body();

     /** Connects a slot to the delete signal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    private:
      mutable DeleteSignal m_delete_signal;
      QWidget* m_body;
  };
}

#endif
