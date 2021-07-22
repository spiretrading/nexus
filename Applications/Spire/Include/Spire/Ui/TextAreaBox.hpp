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

      bool is_read_only() const;

      void set_read_only(bool read_only);

      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
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
      QTextEdit* m_text_edit;
      ScrollBox* m_scroll_box;
      StyleProperties m_text_edit_styles;
      boost::signals2::scoped_connection m_current_connection;
      QString m_submission;
      int m_longest_line_length;
      int m_longest_line_block;
      int m_line_height;

      void commit_style();
      QSize compute_decoration_size() const;
      int get_text_length(const QString& text);
      bool is_scroll_bar_visible() const;
      int line_count() const;
      void update_text_width();
      int visible_line_count() const;
      void on_contents_changed(int position, int removed, int added);
      void on_current(const QString& current);
      void on_cursor_position();
      void on_document_size(const QSizeF& size);
      void on_style();
  };
}

#endif
