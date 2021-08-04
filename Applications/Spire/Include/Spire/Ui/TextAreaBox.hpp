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
          explicit ContentSizedTextEdit(QWidget* parent = nullptr)
              : QTextEdit(parent) {
            setLineWrapMode(QTextEdit::WidgetWidth);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            document()->setDocumentMargin(0);
            setFrameShape(QFrame::NoFrame);
            connect(
              this, &QTextEdit::textChanged, this, [=] { on_text_changed(); });
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
          }

          explicit ContentSizedTextEdit(
              const QString& text, QWidget* parent = nullptr)
              : ContentSizedTextEdit(parent) {
            setText(text);
          }

          void set_read_only(bool read_only) {
            if(read_only != isReadOnly()) {
              setReadOnly(read_only);
              updateGeometry();
            }
          }

          QSize sizeHint() const override {
            auto margins = contentsMargins();
            auto desired_size = QSize(get_longest_line(),
              document()->size().toSize().height());
            auto size = desired_size + QSize(
              margins.left() + margins.right(), margins.top() +
              margins.bottom());
            if(!isReadOnly()) {
              size.rwidth() += cursorWidth();
            }
            return size;
          }

          QSize minimumSizeHint() const override {
            return QSize();
          }

        private:
          void on_text_changed() {
            updateGeometry();
          }

          int get_longest_line() const {
            auto longest = 0;
            for(auto i = 0; i < document()->blockCount(); ++i) {
              auto block = document()->findBlockByNumber(i);
              if(block.isValid()) {
                longest = std::max(longest,
                  fontMetrics().horizontalAdvance(block.text()));
              }
            }
            return longest;
          }
      };

      class ElidedLabel : public QFrame {
        public:

          explicit ElidedLabel(QString text, QWidget *parent = nullptr)
            : QFrame(parent),
              m_text(std::move(text)),
              m_alignment(Qt::AlignLeft) {}

          void set_text(const QString &text) {
            m_text = text;
            update();
          }

          void set_padding(const QMargins& padding) {
            m_padding = padding;
            update();
          }

          void set_text_color(const QColor& color) {
            m_text_color = color;
            update();
          }

          void set_alignment(Qt::Alignment alignment) {
            m_alignment = alignment;
            update();
          }

          QSize sizeHint() const override {
            return {width(), height()};
          }

        protected:
          void paintEvent(QPaintEvent *event) override {
            QPainter painter(this);
            painter.setPen(m_text_color);
            painter.setFont(font());
            QFontMetrics fontMetrics = painter.fontMetrics();
            int lineSpacing = fontMetrics.lineSpacing();
            int y = m_padding.top();
            QTextLayout textLayout(m_text, painter.font());
            auto opt = textLayout.textOption();
            opt.setAlignment(m_alignment);
            textLayout.setTextOption(opt);
            textLayout.beginLayout();
            while(true) {
              QTextLine line = textLayout.createLine();

              if (!line.isValid())
                break;

              line.setLineWidth(width() - m_padding.left() -
                m_padding.right());
              int nextLineY = y + lineSpacing;

              if (height() >= nextLineY + lineSpacing) {
                line.draw(&painter, QPoint(m_padding.left(), y));
                y = nextLineY;
              } else {
                QString lastLine = m_text.mid(line.textStart());
                QString elidedLastLine = fontMetrics.elidedText(lastLine,
                  Qt::ElideRight, width() - m_padding.left() -
                  m_padding.right());
                painter.drawText(QPoint(m_padding.left(),
                  y + fontMetrics.ascent()), elidedLastLine);
                line = textLayout.createLine();
                break;
              }
            }
            textLayout.endLayout();
          }

        private:
          QMargins m_padding;
          QString m_text;
          Qt::Alignment m_alignment;
          QColor m_text_color;
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
      int m_longest_line_length;
      int m_longest_line_block;
      int m_line_height;
      // TODO: remove when selector fixed
      bool m_is_read_only;

      void commit_placeholder_style();
      void commit_style();
      QSize compute_decoration_size() const;
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
