#ifndef SPIRE_TEXT_AREA_BOX_HPP
#define SPIRE_TEXT_AREA_BOX_HPP
#include <boost/optional/optional.hpp>
#include <QStackedWidget>
#include <QTextEdit>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StyleSheetMap.hpp"
#include "Spire/Ui/TextBox.hpp"

namespace Spire {
namespace Styles {

  /** Styles the line height as a percentage of the font height. */
  using LineHeight = BasicProperty<double, struct LineHeightTag>;
}

  /** Displays a multi-line text input box. */
  class TextAreaBox : public QWidget {
    public:

      /**
       * Signals that the current value is being submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const QString& submission)>;

      /**
       * Constructs a TextAreaBox using a LocalTextModel.
       * @param parent The parent widget.
       */
      explicit TextAreaBox(QWidget* parent = nullptr);

      /**
       * Constructs a TextAreaBox using a LocalTextModel and initial current
       * value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit TextAreaBox(QString current, QWidget* parent = nullptr);

      /**
       * Constructs a TextAreaBox.
       * @param model The current value's model.
       * @param parent The parent widget.
       */
      explicit TextAreaBox(std::shared_ptr<TextModel> model,
        QWidget* parent = nullptr);

      /** Returns the model. */
      const std::shared_ptr<TextModel>& get_model() const;

      /** Returns the last submitted value. */
      const QString& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      /** Returns <code>true</code> iff this box is read-only. */
      bool is_read_only() const;

      /** Sets whether the box is read-only. */
      void set_read_only(bool read_only);

      /** Connects a slot to the SubmitSignal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      class ContentSizedTextEdit;
      class ElidedLabel;
      struct StyleProperties {
        Styles::StyleSheetMap m_styles;
        QMargins m_border_sizes;
        QMargins m_padding;
        Qt::Alignment m_alignment;
        QFont m_font;
        boost::optional<int> m_size;
        QColor m_color;
        boost::optional<double> m_line_height;

        StyleProperties(std::function<void ()> commit);
        void clear();
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TextModel> m_model;
      QStackedWidget* m_stacked_widget;
      ContentSizedTextEdit* m_text_edit;
      StyleProperties m_text_edit_styles;
      ElidedLabel* m_placeholder;
      StyleProperties m_placeholder_styles;
      ScrollBox* m_scroll_box;
      boost::signals2::scoped_connection m_current_connection;
      QString m_submission;
      QString m_placeholder_text;
      int m_computed_line_height;

      void apply_block_formatting(
        const std::function<void(const QTextBlock& block)>& formatting);
      void commit_placeholder_style();
      void commit_style();
      QSize get_border_size() const;
      QSize get_padding_size() const;
      bool is_placeholder_shown() const;
      void update_display_text();
      void update_document_line_height();
      //void update_layout();
      void update_placeholder_text();
      void update_text_alignment();
      void update_text_edit_width();
      void on_current(const QString& current);
      void on_cursor_position();
      void on_style();
      void on_text_changed();
  };
}

#endif
