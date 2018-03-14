#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <QWidget>
#include "spire/toolbar/recently_closed_model.hpp"
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! \brief Displays the toolbar window.
  class toolbar_window : public QWidget {
    public:

      //! Signals that this window has closed.
      using closed_signal = signal<void ()>;

      //! Signals that a recently closed window should be re-opened.
      using reopen_signal =
        signal<void (const recently_closed_model::entry& w)>;

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

      //! Connects a slot to the reopen window signal.
      boost::signals2::connection connect_reopen_signal(
        const reopen_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable closed_signal m_closed_signal;
      mutable reopen_signal m_reopen_signal;
  };
}

#endif
