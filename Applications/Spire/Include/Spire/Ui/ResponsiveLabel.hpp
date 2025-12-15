#ifndef SPIRE_RESPONSIVE_LABEL_HPP
#define SPIRE_RESPONSIVE_LABEL_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/signals2/connection.hpp>
#include <QFont>
#include <QSize>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/TextBox.hpp"

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
      explicit ResponsiveLabel(
        std::vector<QString> labels, QWidget* parent = nullptr);

      /**
       * Constructs a ResponsiveLabel.
       * @param list The list of labels to display.
       * @param parent The parent widget.
       */
      explicit ResponsiveLabel(
        std::shared_ptr<ListModel<QString>> labels, QWidget* parent = nullptr);

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
      struct MappedLabel {
        int m_list_index;
        int m_pixel_width;
      };
      std::shared_ptr<ListModel<QString>> m_labels;
      boost::signals2::scoped_connection m_list_operation_connection;
      TextBox* m_text_box;
      std::shared_ptr<LocalTextModel> m_text_model;
      boost::signals2::scoped_connection m_style_connection;
      QSize m_size_hint;
      QFont m_text_box_font;
      std::vector<MappedLabel> m_mapped_labels;
      boost::optional<int> m_current_mapped_index;

      int get_current_label_pixel_width() const;
      int get_next_label_pixel_width() const;
      int get_pixel_width(const QString& text) const;
      bool is_outside_current_bounds(int width) const;
      void reset_mapped_labels();
      void set_current(const boost::optional<int>& mapped_index);
      void sort_mapped_labels();
      void update_display_text();
      void update_size_hint();
      void on_label_added(int index);
      void on_label_removed(int index);
      void on_label_updated(int index);
      void on_list_operation(const AnyListModel::Operation& operation);
      void on_text_box_style();
  };
}

#endif
