#ifndef SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#define SPIRE_CHARTING_TECHNICALS_PANEL_HPP
#include <QLabel>
#include <QWidget>
#include "Spire/Spire/QtPromise.hpp"
#include "Spire/Spire/Signal.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Displays a charting window's technical data in a responsive panel.
  class ChartingTechnicalsPanel : public QWidget {
    public:

      //! Constructs a ChartingTechnicalsPanel with the provided model.
      /*!
        \param model The data source.
      */
      ChartingTechnicalsPanel(TechnicalsModel& model);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      TechnicalsModel* m_model;
      QtPromise<void> m_model_load_promise;
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
      boost::signals2::scoped_connection m_last_price_connection;
      boost::signals2::scoped_connection m_open_connection;
      boost::signals2::scoped_connection m_close_connection;
      boost::signals2::scoped_connection m_high_connection;
      boost::signals2::scoped_connection m_low_connection;
      boost::signals2::scoped_connection m_volume_connection;

      void on_last_price_signal(Nexus::Money last);
      void on_open_signal(Nexus::Money open);
      void on_close_signal(Nexus::Money close);
      void on_high_signal(Nexus::Money high);
      void on_low_signal(Nexus::Money low);
      void on_volume_signal(Nexus::Quantity volume);
      void update_change_label();
  };
}

#endif
