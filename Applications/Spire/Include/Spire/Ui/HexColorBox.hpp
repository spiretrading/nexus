#ifndef SPIRE_HEX_COLOR_BOX_HPP
#define SPIRE_HEX_COLOR_BOX_HPP
#include <memory>
#include <QColor>
#include <QWidget>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Represents a color in hex rgb format #RRGGBB. */
  class HexColorBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QColor& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const QColor& value)>;

      /**
       * Constructs a HexColorBox with a local model.
       * @param parent The parent widget.
       */
      explicit HexColorBox(QWidget* parent = nullptr);

      /**
       * Constructs a HexColorBox using an initial current value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit HexColorBox(QColor current, QWidget* parent = nullptr);

      /**
       * Constructs a HexColorBox.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit HexColorBox(std::shared_ptr<ValueModel<QColor>> current,
        QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<ValueModel<QColor>>& get_current() const;

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct ColorToTextModel;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<ColorToTextModel> m_adaptor_model;
      TextBox* m_text_box;
      QColor m_submission;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_reject_connection;

      void on_submit(const QString& submission);
      void on_reject(const QString& value);
  };
}

#endif
