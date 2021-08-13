#ifndef SPIRE_DESTINATION_LIST_ITEM_HPP
#define SPIRE_DESTINATION_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/Destination.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a destination list item in the DestinationBox. */
  class DestinationListItem : public QWidget {
    public:

      /**
       * Constructs a DestinationListItem.
       * @param destination The destination to be displayed in the
       *                    DestinationListItem.
       * @param parent The parent widget.
       */
      explicit DestinationListItem(
        Nexus::DestinationDatabase::Entry destination,
        QWidget* parent = nullptr);

      /** Returns the destination. */
      const Nexus::DestinationDatabase::Entry& get_destination() const;

    private:
      Nexus::DestinationDatabase::Entry m_destination;
  };
}

#endif
