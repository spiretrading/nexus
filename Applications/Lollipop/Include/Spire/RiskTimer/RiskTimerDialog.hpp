#ifndef SPIRE_RISKTIMERDIALOG_HPP
#define SPIRE_RISKTIMERDIALOG_HPP
#include <memory>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/signals2/connection.hpp>
#include <QDialog>
#include "Spire/RiskTimer/RiskTimer.hpp"

class Ui_RiskTimerDialog;

namespace Spire {

  /*! \class RiskTimerDialog
      \brief Displays the countdown to a RiskState transition.
   */
  class RiskTimerDialog : public QDialog {
    public:

      //! Constructs a RiskTimerDialog.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      RiskTimerDialog(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~RiskTimerDialog();

      //! Sets the model keeping track of the countdown.
      /*!
        \param model The model to display.
      */
      void SetModel(const std::shared_ptr<RiskTimerModel>& model);

    private:
      std::unique_ptr<Ui_RiskTimerDialog> m_ui;
      std::shared_ptr<RiskTimerModel> m_model;
      boost::signals2::scoped_connection m_timeRemainingConnection;

      void OnTimeRemainingUpdated(const boost::posix_time::time_duration&
        timeRemaining);
  };
}

#endif
