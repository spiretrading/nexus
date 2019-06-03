#ifndef SPIRE_RECENTLY_CLOSED_MODEL_HPP
#define SPIRE_RECENTLY_CLOSED_MODEL_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Spire/Toolbar/Toolbar.hpp"

namespace Spire {

  //! Models the recently closed windows that can be re-opened.
  class RecentlyClosedModel : private boost::noncopyable {
    public:

      //! The types of windows that can be re-opened.
      enum class Type {
        BOOK_VIEW,
        TIME_AND_SALE
      };

      //! Stores the details of a single recently closed window.
      struct Entry {

        //! The entry's unique id.
        int m_id;

        //! The type of window that was recently closed.
        Type m_type;

        //! The window's name/identifier.
        std::string m_identifier;
      };

      //! Signals an entry was added to the model.
      /*!
        \param e The entry that was added.
      */
      using EntryAddedSignal = Signal<void (const Entry& e)>;

      //! Signals an entry was removed from the model.
      /*!
        \param e The entry that was removed.
      */
      using EntryRemovedSignal = Signal<void (const Entry& e)>;

      //! Constructs an empty model.
      RecentlyClosedModel();

      //! Returns the list of entries.
      const std::vector<Entry>& get_entries() const noexcept;

      //! Adds a recently closed window entry.
      /*!
        \param t The type of window closed.
        \param identifier The window's identifier.
        \return The entry that was added.
      */
      Entry add(Type t, std::string identifier);

      //! Removes an entry from this model.
      void remove(const Entry& e);

      //! Connects a slot to the entry added signal.
      boost::signals2::connection connect_entry_added_signal(
        const EntryAddedSignal::slot_type& slot) const;

      //! Connects a slot to the entry removed signal.
      boost::signals2::connection connect_entry_removed_signal(
        const EntryRemovedSignal::slot_type& slot) const;

    private:
      mutable EntryAddedSignal m_entry_added_signal;
      mutable EntryRemovedSignal m_entry_removed_signal;
      int m_next_id;
      std::vector<Entry> m_entries;
  };
}

#endif
