#ifndef SPIRE_FLAT_BUTTON_HPP
#define SPIRE_FLAT_BUTTON_HPP
#include <boost/signals2/connection.hpp>
#include <boost/signals2/dummy_mutex.hpp>
#include <boost/signals2/signal_type.hpp>
#include <QFont>
#include <QLabel>
#include <QWidget>

namespace spire {

  //! Displays a flat button with no click animation.
  class flat_button : public QWidget {
    public:

      //! Signals that the button was clicked.
      using clicked_signal = boost::signals2::signal_type<void (),
        boost::signals2::keywords::mutex_type<
        boost::signals2::dummy_mutex>>::type;

      //! Constructs the FlatButton.
      /*!
        \param label The label text.
        \param parent The parent widget to the flat_button.
      */
      flat_button(const QString& label, QWidget* parent = nullptr);

      //! Connects a slot to the clicked signal.
      boost::signals2::connection connect_clicked_signal(
        const clicked_signal::slot_type& slot) const;

    protected:

      virtual void changeEvent(QEvent* event) override;

      virtual void mouseReleaseEvent(QMouseEvent* event) override;

    private:
      QLabel* m_label;
      mutable clicked_signal m_clicked_signal;
  };
}

#endif
