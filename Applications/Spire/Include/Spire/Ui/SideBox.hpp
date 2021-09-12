#ifndef SPIRE_SIDE_BOX_HPP
#define SPIRE_SIDE_BOX_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/EnumBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {  

  /** A ValueModel over a Nexus::Side. */
  using SideModel = ValueModel<Nexus::Side>;

  /** A LocalValueModel over a Nexus::Side. */
  using LocalSideModel = LocalValueModel<Nexus::Side>;
  
  /**
   * Displays a DropDownBox which allows the user to choose a Nexus::Side.
   */
  class SideBox : public EnumBox<Nexus::Side> {
    public:

      /**
       * Constructs a SideBox using a LocalSideModel.
       * @param parent The parent widget.
       */
      explicit SideBox(QWidget* parent = nullptr);

      /**
       * Constructs a SideBox using a LocalSideModel and initial current value.
       * @param current The initial current value.
       * @param parent The parent widget.
       */
      explicit SideBox(Nexus::Side current, QWidget* parent = nullptr);

      /**
       * Constructs a SideBox.
       * @param parent The parent widget.
       */
      explicit SideBox(std::shared_ptr<SideModel> current,
        QWidget* parent = nullptr);

    private:
      Settings setup(std::shared_ptr<SideModel> current);
  };
}

#endif
