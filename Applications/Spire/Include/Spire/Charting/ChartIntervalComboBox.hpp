#ifndef SPIRE_CHARTINTERVALCOMBOBOX_HPP
#define SPIRE_CHARTINTERVALCOMBOBOX_HPP
#include <memory>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/ChartValue.hpp"
#include "Spire/LegacyUI/PersistentWindow.hpp"

class Ui_ChartIntervalComboBox;

namespace Spire {

  /*! \class ChartIntervalComboBox
      \brief Displays a combo box for selecting charting intervals.
   */
  class ChartIntervalComboBox :
      public QWidget, public LegacyUI::PersistentWindow {
    public:

      //! Signals a change to the interval.
      /*!
        \param type The type of interval.
        \param value The interval's value.
      */
      typedef boost::signals2::signal<void (
        const std::shared_ptr<NativeType>& type, ChartValue value)>
        IntervalChangedSignal;

      //! Constructs a ChartIntervalComboBox.
      /*!
        \param parent The parent widget.
        \param flags Qt flags passed to the parent widget.
      */
      ChartIntervalComboBox(QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::WindowFlags());

      virtual ~ChartIntervalComboBox();

      //! Returns the interval's type.
      const NativeType& GetType() const;

      //! Returns the interval's value.
      ChartValue GetValue() const;

      //! Sets the interval.
      /*!
        \param type The interval's type.
        \param value The interval's value.
      */
      void SetInterval(std::shared_ptr<NativeType> type, ChartValue value);

      //! Sets the interval to a time duration.
      /*!
        \param value The interval's value.
      */
      void SetInterval(boost::posix_time::time_duration value);

      //! Connects a slot to the
      boost::signals2::connection ConnectIntervalChangedSignal(
        const IntervalChangedSignal::slot_function_type& slot) const;

      virtual std::unique_ptr<LegacyUI::WindowSettings>
        GetWindowSettings() const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);

    private:
      struct IntervalEntry {
        std::string m_name;
        ChartValue m_value;

        IntervalEntry(boost::posix_time::time_duration value);
      };
      std::unique_ptr<Ui_ChartIntervalComboBox> m_ui;
      std::shared_ptr<NativeType> m_type;
      ChartValue m_value;
      std::vector<IntervalEntry> m_entries;
      mutable IntervalChangedSignal m_intervalChangedSignal;

      std::vector<IntervalEntry> GetStandardDurationIntervals() const;
      void OnCurrentIndexChanged(int index);
  };
}

#endif
