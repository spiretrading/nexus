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

      void set_placeholder(const QString& value);

      bool is_read_only() const;

      void set_read_only(bool read_only);

      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void changeEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      //struct TextEditInfo {
      //  bool m_is_scroll_bar_visible;
      //  QSize
      //};

      class TextEdit : public QTextEdit {
        public:

          explicit TextEdit(QWidget* t_a_b, QWidget *parent = nullptr)
              : QTextEdit(parent),
                m_t_a_b(t_a_b),
                m_longest_line_length(0),
                m_is_scroll_bar_visible(false) {
            //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
            //setWordWrapMode(QTextOption::NoWrap);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            connect(this, &QTextEdit::textChanged, this,
              &TextEdit::on_text_changed);
            connect(document()->documentLayout(),
              &QAbstractTextDocumentLayout::documentSizeChanged, this,
              &TextEdit::on_document_size);
            connect(document(), &QTextDocument::contentsChange, this,
              &TextEdit::on_contents_changed);
            //parent->installEventFilter(this);
            m_t_a_b->installEventFilter(this);
          }

          void set_decoration_size(const QSize& size) {
            m_decoration_size = size;
            updateGeometry();
          }

          void set_scroll_bar_visible(bool is_visible) {
            m_is_scroll_bar_visible = is_visible;
            if(m_is_scroll_bar_visible) {
              //setViewportMargins(0, 0, 0, 0);
            } else {
              //setViewportMargins({});
            }
            //updateGeometry();
          }

          QSize sizeHint() const override {
            // TODO: add decorations, get line height
            return {std::max(scale_width(10), m_longest_line_length + 2),
              line_count() * 15 + 2};
          }

        protected:
          bool eventFilter(QObject* watched, QEvent* event) override {
            if(event->type() == QEvent::Resize) {
              update_page_size();
            }
            return QTextEdit::eventFilter(watched, event);
          }

        private:

          QWidget* m_t_a_b;
          int m_longest_line_length;
          int m_longest_line_block;
          bool m_is_scroll_bar_visible;
          QSize m_decoration_size;

          int get_text_length(const QString& text) {
            // TODO: assumes cursor is always visible (+ 1).
            return fontMetrics().horizontalAdvance(text) + 1;
          }

          QSize get_size_adjustment() const {
            if(!m_is_scroll_bar_visible) {
              return m_decoration_size + QSize(2, 2);
            }
            return m_decoration_size + QSize(15, 0) + QSize(2, 2);
          }

          int line_count() const {
            //auto count = 0;
            //QStringList ret;
            //QTextBlock tb = document()->begin();
            //while(tb.isValid())
            //{
            //  QString blockText = tb.text();
            //  //Q_ASSERT(tb.layout());
            //  if(!tb.layout())
            //    continue;
            //  for(int i = 0; i != tb.layout()->lineCount(); ++i)
            //  {
            //    QTextLine line = tb.layout()->lineAt(i);
            //    ret.append(blockText.mid(line.textStart(), line.textLength()));
            //  }
            //  tb = tb.next();
            //}
            //return ret.size();
            auto num = 0;
            for(auto i = 0; i < document()->blockCount(); ++i) {
              auto block = document()->findBlockByNumber(i);
              if(block.isValid()) {
                num += block.layout()->lineCount();
              }
            }
            //qDebug() << "lc: " << num;
            return std::max(1, num);
          }

          int visible_line_count() const {
            // TODO: get line height
            return (m_t_a_b->height()// -
              /*compute_decoration_size().height()*/) / 15;
          }

          bool is_scroll_bar_visible() {
            qDebug() << "lc: " << line_count();
            qDebug() << "vlc: " << visible_line_count();
            return line_count() > visible_line_count();
          }

          void update_page_size() {
            auto scroll_bar_width = [&] {
              if(is_scroll_bar_visible()) {
                qDebug() << "sb is vis";
                return scale_width(15);
              }
              return 0;
            }();
            if(m_t_a_b->width() < m_longest_line_length) {
              //qDebug() << "page size = tab width";
              document()->setPageSize({
                static_cast<double>(m_t_a_b->width() - scroll_bar_width),
                document()->pageSize().height()});
            } else {
              //qDebug() << "page size = longest line";
              document()->setPageSize({
                static_cast<double>(m_longest_line_length - scroll_bar_width),
                document()->pageSize().height()});
            }
          }

          void on_contents_changed(int position, int charsRemoved,
              int charsAdded) {
            auto block = document()->findBlock(position);
            if(block.isValid()) {
              auto line_length = get_text_length(block.text());
              if(line_length > m_longest_line_length) {
                m_longest_line_length = line_length;
                m_longest_line_block = block.blockNumber();
                setFixedSize(m_longest_line_length, height());
              } else if(block.blockNumber() == m_longest_line_block) {
                m_longest_line_length = 0;
                m_longest_line_block = 0;
                for(auto i = 0; i < document()->blockCount(); ++i) {
                  auto block = document()->findBlockByNumber(i);
                  if(block.isValid()) {
                    if(get_text_length(block.text()) > m_longest_line_length) {
                      m_longest_line_length = get_text_length(block.text());
                      m_longest_line_block = block.blockNumber();
                    }
                  }
                }
              }
            }
            update_page_size();
          }

          void on_document_size(const QSizeF& size) {
            qDebug() << "document size: " << document()->size();
            //setGeometry(0, 0, size.toSize().width(), size.toSize().height());
            setFixedSize(size.toSize());// + QSize(2, 2));
          }

          void on_text_changed() {
            //if(parentWidget()->height() < height()) {
            //  if(!m_is_scroll_bar_visible) {
            //    m_is_scroll_bar_visible = true;
            //    auto page_size = document()->pageSize();
            //    document()->setPageSize({page_size.width() + 15.0,
            //      page_size.height()});
            //  }
            //}
          }
      };

      struct StyleProperties {
        Styles::StyleSheetMap m_styles;
        boost::optional<Qt::Alignment> m_alignment;
        boost::optional<QFont> m_font;
        boost::optional<int> m_size;
        boost::optional<double> m_line_height;

        void clear();
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<TextModel> m_model;
      Box* m_container_box;
      TextEdit* m_text_edit;
      StyleProperties m_text_edit_styles;
      QLabel* m_placeholder;
      StyleProperties m_placeholder_styles;
      boost::signals2::scoped_connection m_current_connection;
      QString m_submission;
      QString m_placeholder_text;
      bool m_is_rejected;
      mutable boost::optional<QSize> m_size_hint;
      int m_line_height;
      int m_max_lines;

      //Box* m_layer_container;
      LayeredWidget* m_layers;
      int m_document_height;
      ScrollBox* m_scroll_box;
      Box* m_text_edit_box;

      QSize compute_decoration_size() const;
      bool is_placeholder_shown() const;
      void elide_current();
      void update_display_text();
      void update_placeholder_text();
      void commit_style();
      void commit_placeholder_style();
      int line_count() const;
      void update_text_box_size();
      int visible_line_count() const;
      void on_current(const QString& current);
      void on_cursor_position();
      void on_editing_finished();
      void on_text_changed();
      void on_style();
  };
}

#endif
