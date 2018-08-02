#ifndef SPIRE_TECHNICALS_PANEL_HPP
#define SPIRE_TECHNICALS_PANEL_HPP
#include <QWidget>
#include <QGridLayout>
#include "spire/book_view/book_view.hpp"
#include "spire/book_view/labeled_data_widget.hpp"

namespace Spire {

  //! Displays a security's technical indicators.
  class TechnicalsPanel : public QWidget {
    public:

      //! Constructs a TechnicalsPanel.
      TechnicalsPanel(QWidget* parent = nullptr);

    protected:
      void resizeEvent(QResizeEvent* event) override;

    private:
      QGridLayout* m_layout;
      LabeledDataWidget* m_high_label_widget;
      LabeledDataWidget* m_open_label_widget;
      LabeledDataWidget* m_defaults_label_widget;
      LabeledDataWidget* m_low_label_widget;
      LabeledDataWidget* m_close_label_widget;
      LabeledDataWidget* m_volume_label_widget;

      void set_labeled_data_long_form_text();
      void set_labeled_data_short_form_text();
  };
}

#endif
