#ifndef SPIRE_SIDE_BOX_HPP
#define SPIRE_SIDE_BOX_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/DropDownBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays a widget for choosing a side from a drop down list.
   */
  class SideBox : public DropDownBoxAdaptor {
    public:

      /**
       * Constructs a SideBox with a LocalValueModel.
       * @param parent The parent widget.
       */
      explicit SideBox(QWidget* parent = nullptr);

      /**
       * Constructs a SideBox.
       * @param current_model The current value's model.
       * @param parent The parent widget.
       */
      explicit SideBox(
        std::shared_ptr<ValueModel<boost::optional<int>>> current_model,
        QWidget* parent = nullptr);
  };
}

#endif
