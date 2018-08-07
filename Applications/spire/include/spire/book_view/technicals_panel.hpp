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

      // Sets the text of the close label.
      /*
        \param close The new string to display.
      */
      void set_close(const QString& close);

      // Sets the text of the high label.
      /*
        \param high The new string to display.
      */
      void set_high(const QString& high);

      // Sets the text of the low label.
      /*
        \param low The new string to display.
      */
      void set_low(const QString& low);

      // Sets the text of the open label.
      /*
        \param open The new string to display.
      */
      void set_open(const QString& open);


      // Sets the text of the volume label.
      /*
        \param volume The new string to display.
      */
      void set_volume(const QString& volume);

      // Resets the text of all labels to their default values.
      void reset_labels();

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
