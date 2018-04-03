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
      using commit_signal = signal<void (const Nexus::Security& s)>;

      //! Signals that this item was hovered by the mouse.
      /*!
        \param widget This, the widget that was hovered.
      */
      using hovered_signal = signal<void (QWidget* widget)>;

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
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;

      //! Connects a slot to the hovered signal.
      boost::signals2::connection connect_hovered_signal(
        const hovered_signal::slot_type& slot) const;

      Nexus::Security get_security();

    protected:
      void mouseReleaseEvent(QMouseEvent* event) override;
      void enterEvent(QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;

    private:
      mutable commit_signal m_commit_signal;
      mutable hovered_signal m_hovered_signal;
      Nexus::Security m_security;
      QLabel* m_security_name_label;
      QLabel* m_company_name_label;
      QLabel* m_icon_label;
  };
}

#endif
