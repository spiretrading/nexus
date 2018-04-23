#ifndef SPIRE_SECURITY_INPUT_WIDGET_HPP
#define SPIRE_SECURITY_INPUT_WIDGET_HPP
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! Displays a input/search box for securities.
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

      //! Constructs a security input box with an initial text value.
      /*!
        \param model The model to query for securities.
        \param initial_text The initial text to display in the line edit.
        \param parent The parent to this widget.
      */
      security_input_box(security_input_model& model,
        const QString& initial_text, QWidget* parent = nullptr);

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const commit_signal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable commit_signal m_commit_signal;
      security_input_model* m_model;
      QLineEdit* m_security_line_edit;
      QLabel* m_icon_label;
      security_info_list_view* m_securities;

      void on_text_edited();
      void move_line_edit();
      void enter_pressed();
      void on_activated(const Nexus::Security& security);
      void on_commit(const Nexus::Security& security);
  };
}

#endif
