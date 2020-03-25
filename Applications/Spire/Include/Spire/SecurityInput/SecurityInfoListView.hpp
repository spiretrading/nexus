#ifndef SPIRE_SECURITY_INFO_LIST_VIEW_HPP
#define SPIRE_SECURITY_INFO_LIST_VIEW_HPP
#include <vector>
#include <QScrollArea>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  //! Displays a list of securities with symbol, exchange, and company name.
  class SecurityInfoListView : public QWidget {
    public:

      //! Signals a security was activated.
      /*!
        \param s The activated security.
      */
      using ActivateSignal = Signal<void (const Nexus::Security& s)>;

      //! Signals a security was committed.
      /*!
        \param s The committed security.
      */
      using CommitSignal = Signal<void (const Nexus::Security& s)>;

      //! Constructs a security info list view with an empty list.
      /*!
        \param parent The parent widget.
      */
      explicit SecurityInfoListView(QWidget* parent = nullptr);

      //! Sets the list of securities to display.
      void set_list(const std::vector<Nexus::SecurityInfo>& list);

      //! Activates the next item in the list.
      void activate_next();

      //! Activates the previous item in the list.
      void activate_previous();

      //! Connects a slot to the activate signal.
      boost::signals2::connection connect_activate_signal(
        const CommitSignal::slot_type& slot) const;

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    private:
      mutable ActivateSignal m_activate_signal;
      mutable CommitSignal m_commit_signal;
      DropShadow* m_shadow;
      QScrollArea* m_scroll_area;
      QWidget* m_list_widget;
      QWidget* m_key_widget;
      int m_highlighted_index;
      int m_active_index;

      void update_active(int active_index);
      void on_highlight(int index, bool is_highlighted);
      void on_commit(const Nexus::Security& security);
  };
}

#endif
