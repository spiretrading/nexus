#ifndef SPIRE_SECURITY_INFO_WIDGET
#define SPIRE_SECURITY_INFO_WIDGET
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! A convenience widget to display a security's name and ticker symbol.
  class security_info_widget : public QWidget {
    public:

      //! Signals that the security represented by this widget should be
      //! committed.
      using commit_signal = signal<void ()>;

      //! Constructs a security_info_widget.
      /*!
        \param info SecurityInfo to display.
        \param parent Parent to this widget.
      */
      security_info_widget(Nexus::SecurityInfo info, QWidget* parent = nullptr);

      //! Returns the security info represented.
      const Nexus::SecurityInfo& get_info() const;

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable commit_signal m_commit_signal;
      Nexus::SecurityInfo m_info;
      QLabel* m_security_name_label;
      QLabel* m_company_name_label;
      QLabel* m_icon_label;

      void display_company_name();
  };
}

#endif
