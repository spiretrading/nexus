#ifndef SPIRE_RECENTLY_CLOSED_MODEL_HPP
#define SPIRE_RECENTLY_CLOSED_MODEL_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! Models the recently closed windows that can be re-opened.
  class recently_closed_model : private boost::noncopyable {
    public:

      //! The types of windows that can be re-opened.
      enum class type {
        BOOK_VIEW,
        TIME_AND_SALE
      };

      //! Stores the details of a single recently closed window.
      struct entry {

        //! The entry's unique id.
        int m_id;

        //! The type of window that was recently closed.
        type m_type;

        //! The window's name/identifier.
        std::string m_identifier;
      };

      //! Signals an entry was added to the model.
      /*!
        \param e The entry that was added.
      */
      using entry_added_signal = Signal<void (const entry& e)>;

      //! Signals an entry was removed from the model.
      /*!
        \param e The entry that was removed.
      */
      using entry_removed_signal = Signal<void (const entry& e)>;

      //! Constructs an empty model.
      recently_closed_model();

      //! Returns the list of entries.
      const std::vector<entry>& get_entries() const noexcept;

      //! Adds a recently closed window entry.
      /*!
        \param t The type of window closed.
        \param identifier The window's identifier.
        \return The entry that was added.
      */
      entry add(type t, std::string identifier);

      //! Removes an entry from this model.
      void remove(const entry& e);

      //! Connects a slot to the entry added signal.
      boost::signals2::connection connect_entry_added_signal(
        const entry_added_signal::slot_type& slot) const;

      //! Connects a slot to the entry removed signal.
      boost::signals2::connection connect_entry_removed_signal(
        const entry_removed_signal::slot_type& slot) const;

    private:
      mutable entry_added_signal m_entry_added_signal;
      mutable entry_removed_signal m_entry_removed_signal;
      int m_next_id;
      std::vector<entry> m_entries;
  };
}

#endif
