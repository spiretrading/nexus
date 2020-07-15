#ifndef SPIRE_DROP_DOWN_LIST_HPP
#define SPIRE_DROP_DOWN_LIST_HPP
#include <QVBoxLayout>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/DropDownItem.hpp"
#include "Spire/Ui/DropDownWindow.hpp"

namespace Spire {

  class DropDownList : public DropDownWindow {
    public:

      using ActivatedSignal = Signal<void (const QVariant& value)>;

      using HighlightedSignal = Signal<void (const QVariant& value)>;

      using SelectedSignal = Signal<void (const QVariant& value)>;

      explicit DropDownList(std::vector<DropDownItem*> items,
        bool is_click_activated, QWidget* parent = nullptr);

      QVariant get_value(int index);

      void set_items(std::vector<DropDownItem*> items);

      boost::signals2::connection connect_activated_signal(
        const ActivatedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event);
      void hideEvent(QHideEvent* event);
      void keyPressEvent(QKeyEvent* event);

    private:
      mutable ActivatedSignal m_activated_signal;
      mutable HighlightedSignal m_highlighted_signal;
      mutable SelectedSignal m_selected_signal;
      int m_max_displayed_items;
      QVBoxLayout* m_layout;
      ScrollArea* m_scroll_area;
      boost::optional<int> m_highlight_index;

      DropDownItem* get_widget(int index);
      void focus_next();
      void focus_previous();
      void set_highlight(int index);
      void scroll_to_highlight();
      void on_item_selected(const QVariant& value);
  };
}

#endif
