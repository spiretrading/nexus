#ifndef SPIRE_SCALABLESCROLLBAR_HPP
#define SPIRE_SCALABLESCROLLBAR_HPP
#include <boost/signals2/signal.hpp>
#include <QTimer>
#include <QWidget>
#include "Spire/LegacyUI/LegacyUI.hpp"

class QFrame;
class QToolButton;
class QWidget;

namespace Spire {
namespace LegacyUI {

  /*! \class ScalableScrollBar
      \brief Implements a scroll bar that can be used to adjust a scale.
   */
  class ScalableScrollBar : public QWidget {
    public:

      //! Signals a change in the range of values.
      /*!
        \param minimum The minimum value.
        \param maximum The maximum value.
      */
      typedef boost::signals2::signal<void (int minimum, int maximum)>
        RangeChangedSignal;

      //! Signals a change in the slider's values.
      /*!
        \param previousMinimum The slider's previous minimum value.
        \param previousMaximum The slider's previous maximum value.
        \param currentMinimum The slider's current minimum value.
        \param currentMaximum The slider's current maximum value.
      */
      typedef boost::signals2::signal<void (int previousMinimum,
        int previousMaximum, int currentMinimum, int currentMaximum)>
        SliderChangedSignal;

      //! Constructs a ScalableScrollBar.
      /*!
        \param parent The parent widget.
      */
      ScalableScrollBar(QWidget* parent = nullptr);

      virtual ~ScalableScrollBar();

      //! Returns <code>true</code> iff the scroll bar is inverted.
      bool IsInverted() const;

      //! Sets whether this scroll bar is inverted.
      void SetInverted(bool inverted);

      //! Returns the minimum value this bar can scroll to.
      int GetMinimum() const;

      //! Sets the minimum value this bar can scroll to.
      void SetMinimum(int minimum);

      //! Returns the maximum value this bar can scroll to.
      int GetMaximum() const;

      //! Sets the maximum value this bar can scroll to.
      void SetMaximum(int maximum);

      //! Returns the slider's current minimum value.
      int GetSliderMinimum() const;

      //! Sets the slider's current minimum value.
      void SetSliderMinimum(int minimum);

      //! Returns the slider's current maximum value.
      int GetSliderMaximum() const;

      //! Sets the slider's current maximum value.
      void SetSliderMaximum(int maximum);

      //! Sets the slider's range of values.
      void SetSliderRange(int minimum, int maximum);

      //! Returns the amount to adjust the handle by in a single step.
      int GetSingleStep() const;

      //! Sets the amount to adjust the handle by in a single step.
      void SetSingleStep(int value);

      //! Returns the amount to adjust the handle by in a page.
      int GetPageStep() const;

      //! Sets the amount to adjust the handle by in a page.
      void SetPageStep(int value);

      //! Returns the scroll bar's orientation.
      Qt::Orientation GetOrientation() const;

      //! Sets the scroll bar's orientation.
      void SetOrientation(Qt::Orientation orientation);

      //! Adjusts the handle by an offset.
      /*!
        \param offset The amount to offset the handle by.
      */
      void AdjustHandle(int offset);

      //! Connects a slot to the RangeChangedSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectRangeChangedSignal(
        const RangeChangedSignal::slot_function_type& slot) const;

      //! Connects a slot to the SliderChangedSignal.
      /*!
        \param slot The slot to connect to the signal.
        \return A connection to the signal.
      */
      boost::signals2::connection ConnectSliderChangedSignal(
        const SliderChangedSignal::slot_function_type& slot) const;

    protected:
      virtual bool eventFilter(QObject* object, QEvent* event);
      virtual void resizeEvent(QResizeEvent* event);

    private:
      QLayout* m_layout;
      QToolButton* m_decrementButton;
      QToolButton* m_incrementButton;
      QWidget* m_scaleBackground;
      QLayout* m_scaleLayout;
      QWidget* m_scaleHandle;
      QFrame* m_bottomScaleHandle;
      QFrame* m_scaleBody;
      QFrame* m_topScaleHandle;
      int m_direction;
      int m_minimum;
      int m_maximum;
      int m_sliderMinimum;
      int m_sliderMaximum;
      int m_singleStep;
      int m_pageStep;
      Qt::Orientation m_orientation;
      QObject* m_mouseTarget;
      int m_mouseValue;
      QTimer m_pageScrollTimer;
      int m_pageScrollDirection;
      mutable RangeChangedSignal m_rangeChangedSignal;
      mutable SliderChangedSignal m_sliderChangedSignal;

      int GetHandleLength() const;
      int PointToScale(const QPoint& point) const;
      QPoint ScaleToPoint(int value) const;
      void Refresh();
      void PageScroll();
      bool ScaleBackgroundMouseEventFilter(QMouseEvent& event);
      bool ScaleHandleMouseEventFilter(QObject& object, QMouseEvent& event);
      void OnPageScrollTimeout();
      void OnIncrementButtonPressed();
      void OnDecrementButtonPressed();
  };
}
}

#endif
