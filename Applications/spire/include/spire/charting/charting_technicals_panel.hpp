#ifndef SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#define SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#include "spire/spire/spire.hpp"
#include <QLabel>
#include <QWidget>

namespace Spire {

  //! Displays a security's technical data in a responsive panel.
  class ChartingTechnicalsPanel : public QWidget {
    public:

      //! Constructs a ChartingTechnicalsPanel with the provided model.
      /*
        \param model The data source.
      */
      ChartingTechnicalsPanel(TechnicalsModel& model);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      TechnicalsModel& m_model;
      QLabel* m_last_label;
      QLabel* m_change_label;
      QLabel* m_open_text_label;
      QLabel* m_open_value_label;
      QLabel* m_close_text_label;
      QLabel* m_close_value_label;
      QLabel* m_high_text_label;
      QLabel* m_high_value_label;
      QLabel* m_low_text_label;
      QLabel* m_low_value_label;
      QLabel* m_volume_text_label;
      QLabel* m_volume_value_label;
  };
}

#endif
