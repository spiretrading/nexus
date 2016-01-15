#ifndef SPIRE_EXPANDBUTTON_HPP
#define SPIRE_EXPANDBUTTON_HPP
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Spire/UI/UI.hpp"

namespace Spire {
namespace UI {

  /*! \class ExpandButton
      \brief Displays an expand/collapse button.
   */
  class ExpandButton : public QWidget {
    public:

      //! Signals that this button was expanded.
      typedef boost::signals2::signal<void ()> ExpandedSignal;

      //! Signals that this button was collapsed.
      typedef boost::signals2::signal<void ()> CollapsedSignal;

      //! Constructs an ExpandButton.
      /*!
        \param parent The parent widget.
      */
      ExpandButton(QWidget* parent = nullptr);

      //! Constructs an ExpandButton.
      /*!
        \param expanded Whether the initial state of this button is expanded.
        \param parent The parent widget.
      */
      ExpandButton(bool expanded, QWidget* parent = nullptr);

      virtual ~ExpandButton();

      //! Returns <code>true</code> iff the state of this button is expanded.
      bool IsExpanded() const;

      //! Sets whether the state of this button is expanded.
      /*!
        \param expanded <code>true</code> iff this button should be in the
               expanded state.
      */
      void SetExpanded(bool expanded);

      //! Connects a slot to the ExpandedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectExpandedSignal(
        const ExpandedSignal::slot_type& slot) const;

      //! Connects a slot to the CollapsedSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectCollapsedSignal(
        const CollapsedSignal::slot_type& slot) const;

    protected:
      virtual void paintEvent(QPaintEvent* event);
      virtual void mousePressEvent(QMouseEvent* event);

    private:
      bool m_expanded;
      mutable ExpandedSignal m_expandedSignal;
      mutable CollapsedSignal m_collapsedSignal;

      void Setup();
  };
}
}

#endif
