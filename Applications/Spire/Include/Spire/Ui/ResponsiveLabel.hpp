#ifndef SPIRE_RESPONSIVE_LABEL_HPP
#define SPIRE_RESPONSIVE_LABEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
  /**
   * Displays a TextBox used as a label whose current value is the longest value
   * among a list of values that will fit within it's body without overflowing.
   * Where two values share the same pixel length, the value with the lowest
   * list index among the same length values is displayed.
   */
  class ResponsiveLabel : public QWidget {
    public:

      /**
       * Constructs a ResponsiveLabel.
       * @param list The list of labels to display.
       * @param parent The parent widget.
       */
      explicit ResponsiveLabel(std::shared_ptr<ListModel<QString>> labels,
        QWidget* parent = nullptr);

      /** Returns the list of labels to display. */
      const std::shared_ptr<ListModel<QString>>& get_labels() const;

      /** Returns the model of the current's displayed value. */
      const std::shared_ptr<TextModel>& get_current() const;

      /** Returns the highlight model. */
      const std::shared_ptr<HighlightModel>& get_highlight() const;

      QSize sizeHint() const override;

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      // TODO: better name
      struct CachedLabel {
        int m_index;
        int m_pixel_width;
      };
      std::shared_ptr<ListModel<QString>> m_labels;
      boost::signals2::connection m_list_operation_connection;
      TextBox* m_text_box;
      std::shared_ptr<LocalTextModel> m_text_model;
      boost::signals2::scoped_connection m_style_connection;
      QFont m_text_box_font;
      std::vector<CachedLabel> m_cached_labels;
      boost::optional<int> m_current_cached_label_index;

      int get_current_label_pixel_width() const;
      int get_next_label_pixel_width() const;
      int get_pixel_width(const QString& text) const;
      bool is_outside_current_bounds(int width) const;
      void reset_cached_labels();
      void set_current(const boost::optional<int> cached_label_index);
      void sort_cached_labels();
      void update_current_font();
      void update_display_text();
      void on_label_added(int index);
      void on_label_removed(int index);
      void on_label_updated(int index);
      void on_list_operation(const AnyListModel::Operation& operation);
      void on_text_box_style();
  };
}

#endif
