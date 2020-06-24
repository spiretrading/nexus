#ifndef SPIRE_SECURITY_INFO_WIDGET_HPP
#define SPIRE_SECURITY_INFO_WIDGET_HPP
#include <QLabel>
#include <QWidget>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Spire/Signal.hpp"

namespace Spire {

  //! A convenience widget to display a security's name and ticker symbol.
  class SecurityInfoWidget : public QWidget {
    public:

      //! Signals whether the represented security is highlighted.
      /*!
        \param value true iff the security is highlighted.
      */
      using HighlightedSignal = Signal<void (bool value)>;

      //! Signals that the represented security should be committed.
      using CommitSignal = Signal<void ()>;

      //! Constructs a SecurityInfoWidget.
      /*!
        \param info SecurityInfo to display.
        \param parent Parent to this widget.
      */
      explicit SecurityInfoWidget(Nexus::SecurityInfo info,
        QWidget* parent = nullptr);

      //! Returns the security info represented.
      const Nexus::SecurityInfo& get_info() const;

      //! Highlights this entry.
      void set_highlighted();

      //! Removes the highlight.
      void remove_highlight();

      //! Connects a slot to the highlighted signal.
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;

      //! Connects a slot to the commit signal.
      boost::signals2::connection connect_commit_signal(
        const CommitSignal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable HighlightedSignal m_highlighted_signal;
      mutable CommitSignal m_commit_signal;
      Nexus::SecurityInfo m_info;
      bool m_is_highlighted;
      QLabel* m_security_name_label;
      QLabel* m_company_name_label;
      QLabel* m_icon_label;

      void display_company_name();
  };
}

#endif
