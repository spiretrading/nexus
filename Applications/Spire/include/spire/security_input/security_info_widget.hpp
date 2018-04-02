#ifndef SPIRE_SECURITY_INFO_WIDGET
#define SPIRE_SECURITY_INFO_WIDGET
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/spire/spire.hpp"

namespace spire {

  //! \brief A convenience widget to display symbol, exchange, and company
  //! name.
  class security_info_widget : public QWidget {
    public:

      //! Signals that this item was selected;
      /*!
        \param s The security that this widget represents.
      */
      using clicked_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a security_info_widget.
      /*!
        \param security The security this widget is representing.
        \param security_name Security name with exchange.
        \param company_name Full company name.
        \param icon_path Path to flag icon to display.
        \param parent Parent to this widget.
      */
      security_info_widget(const Nexus::SecurityInfo& security_info,
        const QString& icon_path, QWidget* parent = nullptr);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    protected:
      void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      mutable clicked_signal m_clicked_signal;
      Nexus::Security m_security;
      QLabel* m_security_name_label;
      QLabel* m_company_name_label;
      QLabel* m_icon_label;
  };
}

#endif
