#ifndef SPIRE_TAG_HPP
#define SPIRE_TAG_HPP
#include <QWidget>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a Tag in a TagBox. */
  class Tag : public QWidget {
    public:

      /**
       * Signals that the tag is being clicked to delete.
       */
      using DeleteSignal = Signal<void ()>;

      /**
       * Constructs a Tag.
       * @param label The text label to display inside of the Tag.
       * @param parent The parent widget.
       */
      explicit Tag(QString label, QWidget* parent = nullptr);

      /** Returns <code>true</code> iff the Tag is not deletable. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the Tag should not be deletable.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

    private:
      bool m_is_read_only;
      Button* m_delete_button;
  };
}

#endif
