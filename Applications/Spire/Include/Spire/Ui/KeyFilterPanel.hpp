#ifndef SPIRE_KEY_FILTER_PANEL_HPP
#define SPIRE_KEY_FILTER_PANEL_HPP
#include "Spire/Ui/KeyListBox.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"

namespace Spire {

  template<>
  struct OpenFilterPanelAdaptor<KeyListBox> {
    using Type = std::shared_ptr<KeySequenceListModel>;

    static bool is_empty(KeyListBox& key_list_box);
    static void clear(KeyListBox& key_list_box);
    static Type get_current(KeyListBox& key_list_box);
    static boost::signals2::connection connect_current(
      KeyListBox& key_list_box, const std::function<void()>& slot);
  };

  extern template class OpenFilterPanel<KeyListBox>;

  /** An OpenFilterPanel specialized for a KeyListBox. */
  using KeyFilterPanel = OpenFilterPanel<KeyListBox>;
}

#endif
