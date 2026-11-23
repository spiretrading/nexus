#ifndef SPIRE_BLOTTER_LINK_MENU_HPP
#define SPIRE_BLOTTER_LINK_MENU_HPP
#include "Spire/Blotter/BlotterCollection.hpp"
#include "Spire/Ui/ContextMenu.hpp"

namespace Spire {

  /** A menu for managing a blotter's links. */
  class BlotterLinkMenu : public ContextMenu {
    public:

      /**
       * Constructs a BlotterLinkMenu.
       * @param model The receiving blotter.
       * @param collection The blotters available for linking.
       * @param parent The parent widget.
       */
      BlotterLinkMenu(std::shared_ptr<BlotterModel> model,
        std::shared_ptr<BlotterCollection> collection, QWidget& parent);

    private:
      struct Entry {
        std::shared_ptr<BlotterModel> m_model;
        std::shared_ptr<BooleanModel> m_checked;
        boost::signals2::scoped_connection m_name_connection;
        boost::signals2::scoped_connection m_checked_connection;
      };
      std::shared_ptr<BlotterModel> m_model;
      std::shared_ptr<BlotterCollection> m_collection;
      std::shared_ptr<const BlottersModel> m_links;
      std::vector<Entry> m_entries;
      boost::signals2::scoped_connection m_collection_connection;
      boost::signals2::scoped_connection m_links_connection;

      void rebuild();
      void on_collection(const BlotterCollection::Operation& operation);
      void on_links(const BlotterCollection::Operation& operation);
  };
}

#endif
