#ifndef SPIRE_SIDE_BOX_HPP
#define SPIRE_SIDE_BOX_HPP
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/DropDownBoxAdaptor.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  template<>
  void DropDownBoxAdaptor<Nexus::Side>::populate_data() {
    m_list_model->push(static_cast<Nexus::Side>(Nexus::Side::ASK));
    m_list_model->push(static_cast<Nexus::Side>(Nexus::Side::BID));
  }

  using SideBox = DropDownBoxAdaptor<Nexus::Side>;
}

#endif
