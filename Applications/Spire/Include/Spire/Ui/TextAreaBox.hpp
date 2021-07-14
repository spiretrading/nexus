#ifndef SPIRE_TEXT_AREA_BOX_HPP
#define SPIRE_TEXT_AREA_BOX_HPP
#include <boost/optional/optional.hpp>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QLabel>
#include <QTextEdit>
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Styles/StyleSheetMap.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/TextBox.hpp"

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

          explicit TextEdit(QWidget *parent = nullptr)
              : QTextEdit(parent),
                m_is_scroll_bar_visible(false) {
            //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
            setWordWrapMode(QTextOption::NoWrap);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            connect(document()->documentLayout(),
              &QAbstractTextDocumentLayout::documentSizeChanged, this,
              &TextEdit::on_document_size);
            //parent->installEventFilter(this);
          }

          void set_decoration_size(const QSize& size) {
            m_decoration_size = size;
            updateGeometry();
          }

          void set_scroll_bar_visible(bool is_visible) {
            m_is_scroll_bar_visible = is_visible;
            if(m_is_scroll_bar_visible) {
              setViewportMargins(0, 0, 0, 0);
            } else {
              //setViewportMargins({});
            }
            //updateGeometry();
          }

          QSize sizeHint() const override {
            return {document()->size().toSize().width(), 300};
          }

        protected:
          //void changeEvent(QEvent* event) override {
          //  if(event->type() == QEvent::ParentChange) {
          //    parent()->installEventFilter(this);
          //  }
          //  QTextEdit::changeEvent(event);
          //}
          //bool eventFilter(QObject* watched, QEvent* event) override {
          //  if(event->type() == QEvent::Resize) {
          //    setFixedSize(parentWidget()->size());
          //    if(parentWidget()->minimumWidth() == parentWidget()->maximumWidth() ||
          //        parentWidget()->sizePolicy().horizontalPolicy() == QSizePolicy::Fixed) {
          //      qDebug() << "wrap";
          //      setWordWrapMode(QTextOption::WordWrap);
          //    } else {
          //      qDebug() << "no wrap";
          //      setWordWrapMode(QTextOption::NoWrap);
          //    }
          //  }
          //  return QTextEdit::eventFilter(watched, event);
          //}

          void resizeEvent (QResizeEvent *event) override {
            /*
              * If the widget has been resized then the size hint will
              * also have changed.  Call updateGeometry to make sure
              * any layouts are notified of the change.
              */
            //updateGeometry();
            //qDebug() << "min: " << minimumWidth();
            //qDebug() << "max: " << maximumWidth();
            //if(minimumWidth() == maximumWidth() ||
            //    sizePolicy().horizontalPolicy() == QSizePolicy::Fixed) {
            //  qDebug() << "wrap";
            //  setWordWrapMode(QTextOption::WordWrap);
            //} else {
            //  qDebug() << "no wrap";
            //  setWordWrapMode(QTextOption::NoWrap);
            //}
            //if(document()->size().toSize() != size()) {
            //  document()->setPageSize(size());
            //}
            QTextEdit::resizeEvent(event);
          }

        private:
          bool m_is_scroll_bar_visible;
          QSize m_decoration_size;

          QSize get_size_adjustment() const {
            if(!m_is_scroll_bar_visible) {
              return m_decoration_size + QSize(2, 2);
            }
            return m_decoration_size + QSize(15, 0) + QSize(2, 2);
          }

          void on_document_size(const QSizeF& size) {
            //setGeometry(0, 0, size.toSize().width(), size.toSize().height());
            //setFixedSize(size.toSize());// + QSize(2, 2));
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
