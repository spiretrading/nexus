#ifndef SPIRE_TOOLBAR_WINDOW_HPP
#define SPIRE_TOOLBAR_WINDOW_HPP
#include <QWidget>
#include "spire/toolbar/toolbar.hpp"

namespace spire {

  //! \brief Displays the toolbar window.
  class toolbar_window : public QWidget {
    public:

      //! Signals that this window has closed.
      using closed_signal = signal<void ()>;

      ~toolbar_window();

      //! Connects a slot to the closed signal.
      boost::signals2::connection connect_closed_signal(
        const closed_signal::slot_type& slot) const;

    protected:
      void closeEvent(QCloseEvent* event) override;

    private:
      mutable closed_signal m_closed_signal;
  };
}

#endif
