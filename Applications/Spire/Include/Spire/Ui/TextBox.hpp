#ifndef SPIRE_TEXT_BOX_HPP
#define SPIRE_TEXT_BOX_HPP
#include <boost/optional/optional.hpp>
#include <QLabel>
#include <QLineEdit>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/BoxGeometry.hpp"
#include "Spire/Ui/BoxPainter.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the element's font. */
  using Font = BasicProperty<QFont, struct FontTag>;

  /** Sets the element's font size. */
  using FontSize = BasicProperty<int, struct FontSizeTag>;

  /** Sets the color of the text. */
  using TextColor = BasicProperty<QColor, struct TextColorTag>;

  /** Styles a widget's text. */
  using TextStyle = CompositeProperty<Font, TextColor>;

  /** Sets the text alignment. */
  using TextAlign = BasicProperty<Qt::Alignment, struct TextAlignTag>;

  /** Selects a a widget whose input value is rejected. */
  using Rejected = StateSelector<void, struct RejectedTag>;

  /** Selects the placeholder. */
  using Placeholder = PseudoElementSelector<void, struct PlaceholderTag>;

  /** Sets the echo mode. */
  using EchoMode = BasicProperty<QLineEdit::EchoMode, struct EchoModeTag>;

  /** Styles a widget's text. */
  TextStyle text_style(QFont font, QColor color);
}

  /** A ValueModel over a QString. */
  using TextModel = ValueModel<QString>;

  /** A ValueModel over an optional QString. */
  using OptionalTextModel = ValueModel<boost::optional<QString>>;

  /** A LocalValueModel over a QString. */
  using LocalTextModel = LocalValueModel<QString>;

  /** A LocalValueModel over an optional QString. */
  using LocalOptionalTextModel = LocalValueModel<boost::optional<QString>>;

  /**
   * Stores the range of text currently highlighted. Can also be used to
   * determine the position of the caret. If the end of the Highlight comes
   * before the start, then the caret is positioned at the beginning of the
   * highlighted range, otherwise the caret is positioned at the end.
   */
  struct Highlight {

    /** The start of the highlight. */
    int m_start;

    /**
     * The end of the highlight (inclusive). Also represents the position of the
     * caret.
     */
    int m_end;

    /** Constructs a Highlight at position 0. */
    Highlight();

    /**
     * Constructs a collapsed Highlight, that is the start and end are the same.
     * @param position The position of the cursor.
     */
    explicit Highlight(int position);

    /** Constructs a Highlight. */
    Highlight(int start, int end);

    auto operator <=>(const Highlight&) const = default;
  };

  /** Returns <code>true</code> iff a highlight's start is equal to its end. */
  bool is_collapsed(const Highlight& highlight);

  /** Returns the size of the selection, including its direction. */
  int get_size(const Highlight& highlight);

  /** A value model over a Highlight. */
  using HighlightModel = ValueModel<Highlight>;

  /** Displays a single line of text within a box. */
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

      /**
       * Constructs a TextBox using a LocalTextModel.
       * @param parent The parent widget.
       */
      explicit TextBox(QWidget* parent = nullptr);

      /**
       * Constructs a TextBox using a LocalTextModel and initial current value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit TextBox(QString current, QWidget* parent = nullptr);

      /**
       * Constructs a TextBox.
       * @param current The current value model.
       * @param parent The parent widget.
       */
      explicit TextBox(
        std::shared_ptr<TextModel> current, QWidget* parent = nullptr);

      /** Returns the current value model. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the last submitted value. */
      const QString& get_submission() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& placeholder);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /** Sets whether the box is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      /** Connects a slot to the RejectedSignal. */
      boost::signals2::connection connect_reject_signal(
        const RejectSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void changeEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct TextStyleProperties {
        Qt::Alignment m_alignment;
        QFont m_font;
        QColor m_text_color;
        QLineEdit::EchoMode m_echo_mode;

        TextStyleProperties();
      };
      struct TextValidator;
      class LineEdit;
      mutable SubmitSignal m_submit_signal;
      mutable RejectSignal m_reject_signal;
      std::shared_ptr<TextModel> m_current;
      std::shared_ptr<TextModel> m_submission;
      QString m_display_text;
      QString m_placeholder;
      bool m_is_read_only;
      std::shared_ptr<HighlightModel> m_highlight;
      TextStyleProperties m_text_style;
      LineEdit* m_line_edit;
      BoxGeometry m_geometry;
      BoxPainter m_box_painter;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_current_connection;
      mutable boost::optional<QSize> m_size_hint;

      void elide_text();
      void initialize_line_edit();
      void update_display_text();
      void on_current(const QString& current);
      void on_style();
      void on_submission(const QString& submission);
  };

  /**
   * Applies a label styling to a widget.
   * @param widget The widget to apply the styling to.
   */
  void apply_label_style(QWidget& widget);

  /**
   * Returns a TextBox as a label.
   * @param label The text displayed on the label.
   * @param parent The parent widget.
   */
  TextBox* make_label(QString label, QWidget* parent = nullptr);

  /**
   * Returns a TextBox as a label using a model.
   * @param current The current value model.
   * @param parent The parent widget.
   */
  TextBox* make_label(
    std::shared_ptr<TextModel> current, QWidget* parent = nullptr);
}

#endif
