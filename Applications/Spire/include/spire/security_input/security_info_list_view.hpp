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

      //! Signals that this item was selected;
      /*!
        \param s The security that this widget represents.
      */
      using clicked_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a security_info_list_view with an empty list.
      /*!
        \param parent The parent to the security_info_list_view.
      */
      security_info_list_view(QWidget* parent = nullptr);

      //! Sets the displayed list.
      /*!
        \param list The list of securities to display.
      */
      void set_list(const std::vector<Nexus::SecurityInfo>& list);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    private:
      mutable clicked_signal m_clicked_signal;
      QWidget* m_list_widget;

      void security_clicked(const Nexus::Security& security);
  };
}

#endif
