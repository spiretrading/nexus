#ifndef SPIRE_TIMERANGEINPUTDIALOG_HPP
#define SPIRE_TIMERANGEINPUTDIALOG_HPP
#include <QDialog>
#include "Spire/InputWidgets/TimeRangeInputWidget.hpp"

class Ui_TimeRangeInputDialog;

namespace Spire {

  /*! \class TimeRangeInputDialog
      \brief Prompts the user for a time range.
   */
  class TimeRangeInputDialog : public QDialog {
    public:

      //! Constructs a TimeRangeInputDialog.
      /*!
        \param startTime The start time.
        \param endTime The end time.
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      TimeRangeInputDialog(const TimeRangeParameter& startTime,
        const TimeRangeParameter& endTime, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~TimeRangeInputDialog();

      //! Returns the start time.
      TimeRangeParameter GetStartTime() const;

      //! Returns the end time.
      TimeRangeParameter GetEndTime() const;

    private:
      std::unique_ptr<Ui_TimeRangeInputDialog> m_ui;
  };
}

#endif
