#ifndef SPIRE_BOOK_VIEW_CONTROLLER_HPP
#define SPIRE_BOOK_VIEW_CONTROLLER_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/connection.hpp>
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

      /**
       * Constructs a BookViewController that already controls an existing
       * BookViewWindow.
       * @param user_profile The user's profile.
       */
      BookViewController(
        Beam::Ref<UserProfile> user_profile, BookViewWindow& window);

      ~BookViewController();

      /** Displays the BookViewWindow. */
      void open();

      /** Closes the BookViewWindow. */
      void close();

      /** Connects a slot to the ClosedSignal. */
      boost::signals2::connection connect_closed_signal(
        const ClosedSignal::slot_type& slot) const;

    private:
      struct EventFilter;
      mutable ClosedSignal m_closed_signal;
      UserProfile* m_user_profile;
      BookViewWindow* m_window;
      std::unique_ptr<EventFilter> m_event_filter;
      boost::signals2::scoped_connection m_submit_task_connection;
      boost::signals2::scoped_connection m_cancel_operation_connection;

      BookViewController(const BookViewController&) = delete;
      BookViewController& operator =(const BookViewController&) = delete;
      void on_submit_task(const std::shared_ptr<CanvasNode>& task);
      void on_cancel_operation(CancelKeyBindingsModel::Operation operation,
        const Nexus::Security& security,
        const boost::optional<BookViewWindow::CancelCriteria>& criteria);
  };
}

#endif
