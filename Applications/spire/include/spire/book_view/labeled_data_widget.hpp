#ifndef SPIRE_LABELED_DATA_WIDGET_HPP
#define SPIRE_LABELED_DATA_WIDGET_HPP
#include <QWidget>
#include <QLabel>

namespace spire {

  //! Represents a label with a label string and data string.
  class labeled_data_widget : public QWidget {
    public:

      //! Constructs a labeled_data_widget with label text and data text.
      /*
        \param label_text The label text string.
        \param data_text The data text value.
        \param parent The parent to this widget.
      */
      labeled_data_widget(const QString& label_text, const QString& data_text,
        QWidget* parent = nullptr);

      //! Sets the label text.
      void set_label_text(const QString& text);

      //! Sets the data text.
      void set_data_text(const QString& text);

    private:
      QLabel* m_label;
      QLabel* m_data_label;
  };
}

#endif
