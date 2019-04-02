#ifndef SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#define SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#include "spire/spire/spire.hpp"

namespace Spire {

  class ChartingTechnicalsPanel : public QWidget {
    public:

      ChartingTechnicalsPanel(std::shared_ptr<TechnicalsModel> model);
  };
}

#endif
