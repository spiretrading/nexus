#ifndef SPIRE_ADDITIONAL_TAGS_WINDOW_HPP
#define SPIRE_ADDITIONAL_TAGS_WINDOW_HPP
#include "Spire/KeyBindings/AdditionalTagsBox.hpp"
#include "Spire/KeyBindings/AdditionalTagDatabase.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/Ui/TextBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  /** Displays a table of additional tags. */
  class AdditionalTagsWindow : public Window {
    public:

      /**
       * Constructs an AdditionalTagsWindow.
       * @param current The list of additional tags to represent.
       * @param additional_tags The definitions of all additional tags.
       * @param destination The destination to constrain the available tags to.
       * @param region The region to constrain the available tags to.
       * @param parent The parent widget.
       */
      AdditionalTagsWindow(std::shared_ptr<AdditionalTagsModel> current,
        AdditionalTagDatabase additional_tags,
        std::shared_ptr<DestinationModel> destination,
        std::shared_ptr<RegionModel> region, QWidget* parent = nullptr);

    private:
      std::shared_ptr<AdditionalTagsModel> m_current;
      AdditionalTagDatabase m_additional_tags;
      std::shared_ptr<DestinationModel> m_destination;
      std::shared_ptr<RegionModel> m_region;
      std::shared_ptr<TableModel> m_tags;
      std::shared_ptr<ListModel<int>> m_available_tags;
      std::shared_ptr<TextModel> m_error_message;

      EditableBox* make_key_item(
        const std::shared_ptr<TableModel>& table, int row, int column) const;
      EditableBox* make_value_item(
        const std::shared_ptr<TableModel>& table, int row, int column) const;
      EditableBox* make_item(
        const std::shared_ptr<TableModel>& table, int row, int column) const;
      void commit();
      void on_cancel();
      void on_done();
  };
}

#endif
