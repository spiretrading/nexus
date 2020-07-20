#ifndef SPIRE_DROP_DOWN_ITEM_HPP
#define SPIRE_DROP_DOWN_ITEM_HPP
#include <QImage>
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  class DropDownItem : public QWidget {
    public:

      using HighlightedSignal = Signal<void (QVariant value)>;

      using SelectedSignal = Signal<void (QVariant value)>;

      explicit DropDownItem(const QVariant& value, QWidget* parent = nullptr);

      DropDownItem(const QVariant& value, const QImage& icon,
        QWidget* parent = nullptr);

      const QVariant& get_value() const;

      void set_highlight();

      void reset_highlight();

      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void enterEvent(QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void paintEvent(QPaintEvent* event) override;

    private:
      QVariant m_value;
      QImage m_icon;
      mutable HighlightedSignal m_highlighted_signal;
      mutable SelectedSignal m_selected_signal;
      bool m_is_highlighted;
      CustomVariantItemDelegate m_item_delegate;
  };
}

#endif
