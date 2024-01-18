#ifndef SPIRE_EYE_DROPPER_HPP
#define SPIRE_EYE_DROPPER_HPP
#include <QTimer>
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/ColorBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** A tool that allows the user to pick a color from on-screen elements. */
  class EyeDropper : public QWidget {
    public:

      /**
       * Signals that the current color is being submitted.
       * @param submission The submitted color.
       */
      using SubmitSignal = Signal<void (const QColor& submission)>;

      /**
       * Signals that the current color is being rejected.
       * @param color The rejected color.
       */
      using RejectSignal = Signal<void (const QColor& color)>;

      /**
       * Constructs an EyeDropper.
       * @param parent The parent widget.
       */
      explicit EyeDropper(QWidget* parent = nullptr);

      /** Returns the current color model. */
      const std::shared_ptr<ColorModel>& get_current() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectSignal. */
      boost::signals2::connection connect_reject_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void keyPressEvent(QKeyEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void showEvent(QShowEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<ColorModel> m_current;
      ClickObserver m_click_observer;
      QImage m_screen_image;
      QRectF m_indicator_geometry;
      QTimer m_timer;

      void on_click();
      void on_timeout();
  };
}

#endif
