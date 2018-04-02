#ifndef SPIRE_SECURITY_INPUT_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_WIDGET_HPP
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! \brief Displays a input/search box for securities.
  class security_input_box : public QWidget {
    public:

      //! Signals that editing has completed.
      /*!
        \param s The security that was input.
      */
      using commit_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a blank security input box.
      /*!
        \param model The model to query for securities.
        \param parent The parent to this widget.
      */
      security_input_box(security_input_model& model,
        QWidget* parent = nullptr);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable commit_signal m_commit_signal;
      security_input_model* m_model;
      QLineEdit* m_security_line_edit;
      QLabel* m_icon_label;
      security_info_list_view* m_securities;

      void security_selected(const Nexus::Security& security);
      void on_text_changed();
      void enter_pressed();
  };
}

#endif
