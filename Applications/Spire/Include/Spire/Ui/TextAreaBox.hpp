#ifndef SPIRE_TEXT_AREA_BOX_HPP
#define SPIRE_TEXT_AREA_BOX_HPP
#include <boost/optional/optional.hpp>
#include <QApplication>
#include <QLabel>
#include <QTextEdit>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StyleSheetMap.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"

#include <QFrame>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QStackedWidget>

namespace Spire {
namespace Styles {

  /** Styles the line height as a percentage of the font height. */
  using LineHeight = BasicProperty<double, struct LineHeightTag>;
}

  class TextAreaBox : public QWidget {
    public:

      using SubmitSignal = Signal<void (const QString& submission)>;

      explicit TextAreaBox(QWidget* parent = nullptr);

      explicit TextAreaBox(QString current, QWidget* parent = nullptr);

      explicit TextAreaBox(std::shared_ptr<TextModel> model,
        QWidget* parent = nullptr);

      const std::shared_ptr<TextModel>& get_model() const;

      const QString& get_submission() const;

      /** Sets the placeholder value. */
      void set_placeholder(const QString& value);

      bool is_read_only() const;

      void set_read_only(bool read_only);

      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* watched, QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      class ContentSizedTextEdit : public QTextEdit {
        public:

          explicit ContentSizedTextEdit(const QString& text,
            QWidget* parent = nullptr);

          QSize sizeHint() const override;

          QSize minimumSizeHint() const override;

        private:
          int m_longest_line_width;

          void on_text_changed();
          int get_longest_line_width() const;
      };
      class ElidedLabel : public QWidget {
        public:

          explicit ElidedLabel(QWidget *parent = nullptr);

          void set_text(const QString &text);

          void set_padding(const QMargins& padding);

          void set_text_color(const QColor& color);

          void set_alignment(Qt::Alignment alignment);

          void set_line_height(int line_height);

          QSize sizeHint() const override;

        protected:
          void paintEvent(QPaintEvent *event) override;

        private:
          QMargins m_padding;
          QString m_text;
          Qt::Alignment m_alignment;
          QColor m_text_color;
          int m_line_height;
      };
      struct StyleProperties {
        Styles::StyleSheetMap m_styles;
        QMargins m_padding;
        boost::optional<Qt::Alignment> m_alignment;
        boost::optional<QFont> m_font;
        boost::optional<int> m_size;
        boost::optional<QColor> m_color;
        boost::optional<double> m_line_height;

        void clear();
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TextModel> m_model;
      QStackedWidget* m_stacked_widget;
      ContentSizedTextEdit* m_text_edit;
      StyleProperties m_text_edit_styles;
      ElidedLabel* m_placeholder;
      ElidedLabel* m_read_only_label;
      StyleProperties m_placeholder_styles;
      ScrollBox* m_scroll_box;
      boost::signals2::scoped_connection m_current_connection;
      QString m_submission;
      QString m_placeholder_text;
      int m_computed_line_height;
      // TODO: remove when selector fixed
      bool m_is_read_only;

      void commit_placeholder_style();
      void commit_style();
      QSize compute_border_size() const;
      QSize compute_padding_size() const;
      bool is_placeholder_shown() const;
      void update_display_text();
      void update_line_height();
      void update_placeholder_text();
      void update_text_alignment(Qt::Alignment alignment);
      void update_text_edit_width();
      void on_current(const QString& current);
      void on_cursor_position();
      void on_style();
      void on_text_changed();
  };
}

#endif
