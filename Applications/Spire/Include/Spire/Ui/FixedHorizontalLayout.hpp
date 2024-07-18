#ifndef SPIRE_FIXED_HORIZONTAL_LAYOUT_HPP
#define SPIRE_FIXED_HORIZONTAL_LAYOUT_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include <QLayout>
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Implements a horizontal box layout where every item is a fixed width. */
  class FixedHorizontalLayout : public QLayout {
    public:

      /**
       * Constructs a FixedHorizontalLayout for a widget.
       * @param parent The widget to layout.
       */
      FixedHorizontalLayout(QWidget* parent = nullptr);

      void addItem(QLayoutItem* item) override;

      QSize sizeHint() const override;

      void setGeometry(const QRect& rect) override;

      QLayoutItem* itemAt(int index) const override;

      QLayoutItem* takeAt(int index) override;

      int count() const override;

      void invalidate() override;

    private:
      mutable boost::optional<QSize> m_size_hint;
      std::vector<std::unique_ptr<QLayoutItem>> m_items;
  };
}

#endif
