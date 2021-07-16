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
      QTextEdit* m_text_edit;
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
      int m_longest_line_length;
      int m_longest_line_block;

      // TODO: alphabetical
      QSize compute_decoration_size() const;
      bool is_placeholder_shown() const;
      bool is_scroll_bar_visible() const;
      void elide_current();
      int get_text_length(const QString& text);
      void update_display_text();
      void update_placeholder_text();
      void commit_style();
      void commit_placeholder_style();
      int line_count() const;
      void update_page_size();
      void update_text_box_size();
      int visible_line_count() const;
      void on_contents_changed(int position, int removed, int added);
      void on_current(const QString& current);
      void on_cursor_position();
      void on_document_size(const QSizeF& size);
      void on_editing_finished();
      void on_text_changed();
      void on_style();
  };
}

#endif
