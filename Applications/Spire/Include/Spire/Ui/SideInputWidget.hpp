#ifndef SPIRE_SIDE_INPUT_WIDGET_HPP
#define SPIRE_SIDE_INPUT_WIDGET_HPP
#include <QWidget>
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  //! Represents a widget for selecting Side values.
  class SideInputWidget : public QWidget {
    public:

      //! Signals that a Side was selected.
      /*1
        \param side The selected Side.
      */
      using SideSignal = Signal<void (const Nexus::Side& side)>;

      //! Constructs a SideInputWidget.
      /*!
        \param parent The parent widget.
      */
      SideInputWidget(QWidget* parent = nullptr);

      //! Connects a slot to the side selection signal.
      boost::signals2::connection connect_side_signal(
        const SideSignal::slot_type& slot) const;

    private:
      mutable SideSignal m_side_signal;
      boost::signals2::scoped_connection m_value_connection;
  };
}

#endif
