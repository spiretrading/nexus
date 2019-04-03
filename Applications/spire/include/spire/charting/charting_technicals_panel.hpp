#ifndef SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#define SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#include "spire/spire/spire.hpp"
#include <QLabel>
#include <QWidget>
#include "spire/ui/custom_qt_variants.hpp"

namespace Spire {

  //! Displays a charting window's technical data in a responsive panel.
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
      CustomVariantItemDelegate* m_item_delegate;
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

      void on_last_price_signal(const Nexus::Money& last);
      void on_open_signal(const Nexus::Money& open);
      void on_close_signal(const Nexus::Money& close);
      void on_high_signal(const Nexus::Money& high);
      void on_low_signal(const Nexus::Money& low);
      void on_volume_signal(const Nexus::Quantity& volume);
      void update_change_label();
  };
}

#endif
