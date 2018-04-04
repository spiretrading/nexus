#ifndef SPIRE_SECURITY_INFO_LIST_VIEW
#define SPIRE_SECURITY_INFO_LIST_VIEW
#include <vector>
#include <QScrollArea>
#include "Nexus/Definitions/Security.hpp"
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/spire/spire.hpp"

namespace spire {

  //! \brief Displays a list of securities with symbol, exchange, and company
  //! name.
  class security_info_list_view : public QScrollArea {
    public:

      //! Signals that an item was selected.
      /*!
        \param s The security that the selected widget represents.
      */
      using selected_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a security_info_list_view with an empty list.
      /*!
        \param key_widget Widget that security_info_listview will receive
               key events from.
        \param parent The parent to the security_info_list_view.
      */
      security_info_list_view(QWidget* key_widget, QWidget* parent = nullptr);

      //! Sets the displayed list.
      /*!
        \param list The list of securities to display.
      */
      void set_list(const std::vector<Nexus::SecurityInfo>& list);

      //! Highlights the first/next item in the list.
      void highlight_next_item();

      //! Highlights the last/previous item in the list.
      void highlight_previous_item();

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const selected_signal::slot_type& slot) const;

      boost::signals2::connection connect_highlighted_signal(
        const selected_signal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void leaveEvent(QEvent* event) override;

    private:
      mutable selected_signal m_commit_signal;
      mutable selected_signal m_highlighted_signal;
      QWidget* m_list_widget;
      QWidget* m_key_widget;
      static const int M_MAX_VISIBLE_ITEMS = 5;
      int m_current_index;
      int m_hover_index;

      void commit(const Nexus::Security& security);
      void update_hover_index(QWidget* widget);
  };
}

#endif
