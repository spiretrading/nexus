#ifndef SPIRE_TAG_HPP
#define SPIRE_TAG_HPP
#include <QWidget>
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/BoxPainter.hpp"

namespace Spire {

  /** Represents a Tag in a TagBox. */
  class Tag : public QWidget {
    public:

      /**
       * Signals that the tag is being clicked to delete.
       */
      using DeleteSignal = Signal<void ()>;

      /**
       * Constructs a Tag.
       * @param label The text label to display inside of the Tag.
       * @param parent The parent widget.
       */
      explicit Tag(QString label, QWidget* parent = nullptr);

      /**
       * Constructs a Tag.
       * @param label The text label to display inside of the Tag.
       * @param parent The parent widget.
       */
      explicit Tag(std::shared_ptr<TextModel> label, QWidget* parent = nullptr);

      /** Returns the text label. */
      const std::shared_ptr<TextModel>& get_label() const;

      /** Returns <code>true</code> iff the Tag is not deletable. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the Tag should not be deletable.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the DeleteSignal. */
      boost::signals2::connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      void mouseMoveEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void leaveEvent(QEvent* event) override;
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

    private:
      mutable DeleteSignal m_delete_signal;
      std::shared_ptr<TextModel> m_label;
      bool m_is_read_only;
      bool m_is_delete_hovered;
      bool m_is_delete_pressed;
      QColor m_background_color;
      int m_border_radius;
      int m_horizontal_padding;
      int m_vertical_padding;
      QColor m_text_color;
      QFont m_font;
      QColor m_delete_fill;
      QImage m_delete_icon;
      boost::signals2::scoped_connection m_label_connection;
      boost::signals2::scoped_connection m_style_connection;

      QRect get_delete_rect() const;
      bool is_delete_visible() const;
      void on_style();
  };
}

#endif
