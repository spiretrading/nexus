#ifndef SPIRE_SECURITY_INFO_LIST_VIEW
#define SPIRE_SECURITY_INFO_LIST_VIEW
#include <vector>
#include <QScrollArea>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! Displays a list of securities with symbol, exchange, and company name.
  class security_info_list_view : public QWidget {
    public:

      //! Signals a security was activated.
      /*!
        \param s The activated security.
      */
      using activate_signal = signal<void (const Nexus::Security& s)>;

      //! Signals a security was committed.
      /*!
        \param s The committed security.
      */
      using commit_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a security info list view with an empty list.
      /*!
        \param parent The parent widget.
      */
      security_info_list_view(QWidget* parent = nullptr);

      //! Sets the list of securities to display.
      void set_list(const std::vector<Nexus::SecurityInfo>& list);

      //! Activates the next item in the list.
      void activate_next();

      //! Activates the previous item in the list.
      void activate_previous();

      //! Connects a slot to the activate signal.
      boost::signals2::connection connect_activate_signal(
        const commit_signal::slot_type& slot) const;

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable commit_signal m_activate_signal;
      mutable commit_signal m_commit_signal;
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
