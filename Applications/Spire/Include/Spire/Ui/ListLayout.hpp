#ifndef SPIRE_LIST_LAYOUT_HPP
#define SPIRE_LIST_LAYOUT_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QLayout>

namespace Spire {
namespace Styles {

  /** Specifies how to layout items on overflow. */
  enum class Overflow {

    /** The list extends indefinitely. */
    NONE,

    /** List items wrap to fill the perpendicular space. */
    WRAP
  };
}

  /** A layout that arranges items along a direction with optional wrapping. */
  class ListLayout : public QLayout {
    public:

      /**
       * Constructs an empty ListLayout.
       * @param parent The widget to install the layout into.
       */
      explicit ListLayout(QWidget* parent = nullptr);

      /** Returns the direction items are laid out in. */
      Qt::Orientation get_direction() const;

      /** Sets the direction items are laid out in. */
      void set_direction(Qt::Orientation direction);

      /** Returns the overflow behavior. */
      Styles::Overflow get_overflow() const;

      /** Sets the overflow behavior. */
      void set_overflow(Styles::Overflow overflow);

      /** Returns the spacing between consecutive items in a row or column. */
      int get_item_gap() const;

      /** Sets the spacing between consecutive items in a row or column. */
      void set_item_gap(int gap);

      /** Returns the spacing between consecutive rows or columns. */
      int get_overflow_gap() const;

      /** Sets the spacing between consecutive rows or columns. */
      void set_overflow_gap(int gap);

      /** Inserts an item at the given index. */
      void insert_item(int index, QLayoutItem& item);

      /** Inserts a widget at the given index. */
      void insert_widget(int index, QWidget& widget);

      void addItem(QLayoutItem* item) override;
      QLayoutItem* itemAt(int index) const override;
      QLayoutItem* takeAt(int index) override;
      int count() const override;
      QSize sizeHint() const override;
      Qt::Orientations expandingDirections() const override;
      bool hasHeightForWidth() const override;
      int heightForWidth(int width) const override;
      void setGeometry(const QRect& rect) override;
      void invalidate() override;

    private:
      Qt::Orientation m_direction;
      Styles::Overflow m_overflow;
      int m_item_gap;
      int m_overflow_gap;
      std::vector<std::unique_ptr<QLayoutItem>> m_items;
      mutable boost::optional<QSize> m_size_hint;

      QSize compute_size(const QRect& rect) const;
      void apply_layout(const QRect& rect);
  };
}

#endif
