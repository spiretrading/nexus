#ifndef SPIRE_SIDE_BOX_HPP
#define SPIRE_SIDE_BOX_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/DropDownBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  template<>
  void DropDownBoxAdaptor<Nexus::Side>::populate_data(
      std::shared_ptr<ArrayListModel> list_model) {
    list_model->push(static_cast<Nexus::Side>(Nexus::Side::ASK));
    list_model->push(static_cast<Nexus::Side>(Nexus::Side::BID));
  }

  using SideBox = DropDownBoxAdaptor<Nexus::Side>;
}

#endif
