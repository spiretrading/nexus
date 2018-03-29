#ifndef SPIRE_SECURITY_INFO_WIDGET
#define SPIRE_SECURITY_INFO_WIDGET
#include <QLabel>
#include <QWidget>

namespace spire {

  //! \brief A convenience widget to display symbol, exchange, and company
  //! name.
  class security_info_widget : public QWidget {
    public:

      //! Constructs a security_info_widget.
      /*!
        \param security_name Security name with exchange.
        \param company_name Full company name.
        \param icon_path Path to flag icon to display.
        \param parent Parent to this widget.
      */
      security_info_widget(const QString& security_name,
        const QString& company_name, const QString& icon_path,
        QWidget* parent = nullptr);

    private:
      QLabel* m_security_name_label;
      QLabel* m_company_name_label;
      QLabel* m_icon_label;
  };
}

#endif
