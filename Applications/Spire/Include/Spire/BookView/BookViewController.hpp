#ifndef SPIRE_BOOK_VIEW_CONTROLLER_HPP
#define SPIRE_BOOK_VIEW_CONTROLLER_HPP
#include <Beam/Pointers/Ref.hpp>
#include "Spire/BookView/BookView.hpp"
#include "Spire/BookView/BookViewWindow.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  /** Implements the application controller for the BookViewWindow. */
  class BookViewController {
    public:

      /** Signals that the window associated with this controller is closed. */
      using ClosedSignal = Signal<void ()>;

      /**
       * Constructs a BookViewController.
       * @param user_profile The user's profile.
       */
      explicit BookViewController(Beam::Ref<UserProfile> user_profile);

      ~BookViewController();

      /** Displays the BookViewWindow. */
      void open();

      /** Closes the BookViewWindow. */
      void close();

      /** Connects a slot to the ClosedSignal. */
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      mutable ClosedSignal m_closed_signal;
      UserProfile* m_user_profile;
      BookViewWindow* m_window;

      BookViewController(const BookViewController&) = delete;
      BookViewController& operator =(const BookViewController&) = delete;
  };
}

#endif
