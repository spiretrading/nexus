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
            setStyleSheet(R"(
              QTextEdit {
                padding: 7px 8px 7px 8px;
              })");
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

      class ElidedLabel : public QWidget {
        public:

          explicit ElidedLabel(QString text, QWidget *parent = nullptr)
            : QWidget(parent),
              m_alignment(Qt::AlignLeft) {}

          void set_lines(const std::vector<QString>& lines) {
            m_lines = lines;
          }

          void set_text(const QString &text) {
            m_lines = {text};
            update();
          }

          Qt::Alignment get_alignment() const {
            return m_alignment;
          }

          void set_alignment(Qt::Alignment alignment) {
            m_alignment = alignment;
          }

          QSize sizeHint() const override {
            return {width(), height()};
          }

        protected:
          void paintEvent(QPaintEvent *event) override {
            QPainter painter(this);
            QFontMetrics fontMetrics = painter.fontMetrics();
            bool didElide = false;
            int lineSpacing = fontMetrics.lineSpacing();
            int y = 0;
            for(const auto& line : m_lines) {
              QTextLayout textLayout(line, painter.font());
              auto opt = textLayout.textOption();
              opt.setAlignment(m_alignment);
              textLayout.setTextOption(opt);
              textLayout.beginLayout();
              while(true) {
                QTextLine text_line = textLayout.createLine();

                if (!text_line.isValid())
                  break;

                text_line.setLineWidth(width());
                int nextLineY = y + lineSpacing;

                if (height() >= nextLineY + lineSpacing) {
                  text_line.draw(&painter, QPoint(0, y));
                  y = nextLineY;
                } else {
                  QString lastLine = line.mid(text_line.textStart());
                  QString elidedLastLine = fontMetrics.elidedText(lastLine,
                    Qt::ElideRight, width());
                  painter.drawText(QPoint(0, y + fontMetrics.ascent()),
                    elidedLastLine);
                  text_line = textLayout.createLine();
                  didElide = text_line.isValid();
                  break;
                }
              }
              textLayout.endLayout();
              //if (didElide != elided) {
              //  elided = didElide;
              //}
            }
          }

        private:
          std::vector<QString> m_lines;
          Qt::Alignment m_alignment;
      };

      struct StyleProperties {
        Styles::StyleSheetMap m_styles;
        boost::optional<QMargins> m_padding;
        boost::optional<Qt::Alignment> m_alignment;
        boost::optional<QFont> m_font;
        boost::optional<int> m_size;
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

      void commit_placeholder_style();
      void commit_style();
      QSize compute_decoration_size() const;
      std::vector<QString> get_current_text_lines() const;
      bool is_placeholder_shown() const;
      void update_display_text();
      void update_placeholder_text();
      void update_text_edit_width();
      void on_current(const QString& current);
      void on_cursor_position();
      void on_style();
      void on_text_changed();
  };
}

#endif
