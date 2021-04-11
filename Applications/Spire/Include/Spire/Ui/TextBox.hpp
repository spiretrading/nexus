#ifndef SPIRE_TEXT_BOX_HPP
#define SPIRE_TEXT_BOX_HPP
#include <QLabel>
#include <QLineEdit>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/ValueModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the element's font. */
  using Font = BasicProperty<QFont, struct FontTag>;

  /** Sets the color of the text. */
  using TextColor = BasicProperty<QColor, struct TextColorTag>;

  /** Styles a widget's text. */
  using TextStyle = CompositeProperty<Font, TextColor>;

  /** Sets the text alignment. */
  using TextAlign = BasicProperty<Qt::Alignment, struct TextAlignTag>;

  /** Selects a read-only widget. */
  using ReadOnly = StateSelector<void, struct ReadOnlyTag>;

  /** Selects the placeholder. */
  using Placeholder = PseudoElementSelector<void, struct PlaceholderTag>;

  /** Styles a widget's text. */
  TextStyle text_style(QFont font, QColor color);
}

  /** The type of model used by the TextBox. */
  using TextModel = ValueModel<QString>;

  //! Displays a one-line text box.
  class TextBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QString& submission)>;

      /**
       * Signals that the current value was rejected as a submission.
       * @param value The value that was rejected.
       */
      using RejectSignal = Signal<void (const QString& value)>;

      //! Constructs a TextBox using a LocalTextModel.
      /*!
        \param parent The parent widget.
      */
      explicit TextBox(QWidget* parent = nullptr);

      //! Constructs a TextBox using a LocalTextModel and initial current value.
      /*!
        \param current The initial current value.
        \param parent The parent widget.
      */
      explicit TextBox(QString current, QWidget* parent = nullptr);

      //! Constructs a TextBox.
      /*!
        \param model The current value's model.
        \param parent The parent widget.
      */
      explicit TextBox(std::shared_ptr<TextModel> model,
        QWidget* parent = nullptr);

      //! Returns the model.
      const std::shared_ptr<TextModel>& get_model() const;

      //! Returns the last submitted value.
      const QString& get_submission() const;

      //! Sets the placeholder value.
      void set_placeholder(const QString& value);

      //! Returns <code>true</code> iff this box is read-only.
      bool is_read_only() const;

      //! Sets whether the box is read-only.
      void set_read_only(bool read_only);

      //! Returns whether a warning is displayed when a submission is rejected.
      bool is_warning_displayed() const;

      //! Sets whether a warning is displayed when a submission is rejected.
      void set_warning_displayed(bool is_displayed);

      //! Connects a slot to the SubmitSignal.
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      //! Connects a slot to the RejectedSignal.
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<TextModel> m_model;
      Box* m_box;
      LayeredWidget* m_layers;
      QLineEdit* m_line_edit;
      QFont m_line_edit_font;
      QLabel* m_placeholder;
      QFont m_placeholder_font;
      bool m_is_warning_displayed;
      boost::signals2::scoped_connection m_current_connection;
      QString m_submission;
      QString m_placeholder_text;

      bool is_placeholder_shown() const;
      QString get_elided_text(const QFontMetrics& font_metrics,
        const QString& text) const;
      void elide_text();
      void update_display_text();
      void update_placeholder_text();
      void on_current(const QString& current);
      void on_editing_finished();
      void on_text_edited(const QString& text);
      void on_style();
      void on_placeholder_style();
  };
}

#endif
