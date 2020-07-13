#ifndef SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#define SPIRE_FILTERED_DROP_DOWN_MENU_HPP
#include <QLineEdit>
#include "Spire/Ui/DropDownList.hpp"

namespace Spire {

  class FilteredDropDownMenu : public QLineEdit {
    public:

      using SelectedSignal = Signal<void (const QVariant& value)>;

      explicit FilteredDropDownMenu(const std::vector<QVariant>& items,
        QWidget* parent = nullptr);

      void set_items(const std::vector<QVariant>& items);

      boost::signals2::connection connect_selected_signal(
        const SelectedSignal::slot_type& slot) const;

    protected:
      void paintEvent(QPaintEvent* event) override;

    private:
      mutable SelectedSignal m_selected_signal;
      DropDownList* m_menu_list;
      QVariant m_current_item;
  };
}

#endif
