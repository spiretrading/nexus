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

      using IndexSelectedSignal = Signal<void (int index)>;

      using ValueSelectedSignal = Signal<void (const QVariant& value)>;

      explicit DropDownList(std::vector<DropDownItem*> items,
        bool is_click_activated, QWidget* parent = nullptr);

      QVariant get_value(int index);

      void insert_item(DropDownItem* item);

      int item_count() const;

      void remove_item(int index);

      void set_items(std::vector<DropDownItem*> items);

      boost::signals2::connection connect_activated_signal(
        const ActivatedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_highlighted_signal(
        const HighlightedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_index_selected_signal(
        const IndexSelectedSignal::slot_type& slot) const;
      
      boost::signals2::connection connect_value_selected_signal(
        const ValueSelectedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event);
      void hideEvent(QHideEvent* event);
      void keyPressEvent(QKeyEvent* event);

    private:
      mutable ActivatedSignal m_activated_signal;
      mutable HighlightedSignal m_highlighted_signal;
      mutable IndexSelectedSignal m_index_selected_signal;
      mutable ValueSelectedSignal m_value_selected_signal;
      int m_max_displayed_items;
      QVBoxLayout* m_layout;
      ScrollArea* m_scroll_area;
      boost::optional<int> m_highlight_index;
      std::vector<boost::signals2::scoped_connection>
        m_item_selected_connections;

      DropDownItem* get_widget(int index);
      void focus_next();
      void focus_previous();
      void set_highlight(int index);
      void scroll_to_highlight();
      void update_height();
      void on_item_selected(const QVariant& value, int index);
  };
}

#endif
