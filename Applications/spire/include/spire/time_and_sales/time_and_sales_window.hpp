#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <boost/optional.hpp>
#include <QWidget>
#include "Nexus/Definitions/Security.hpp"
#include "spire/time_and_sales/time_and_sales.hpp"
#include "spire/time_and_sales/time_and_sales_properties.hpp"
#include "spire/time_and_sales/time_and_sales_window_model.hpp"

namespace spire {

  //! Displays a time and sales window.
  class time_and_sales_window : public QWidget {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using change_security_signal = signal<void (const Nexus::Security& s)>;

      //! Constructs a time and sales window.
      /*!
        \param properties The display properties.
        \param parent The parent widget.
      */
      time_and_sales_window(const time_and_sales_properties& properties,
        QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<time_and_sales_model> model);

      //! Returns the display properties.
      const time_and_sales_properties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const time_and_sales_properties& properties);

    private:
      mutable change_security_signal m_change_security_signal;
      time_and_sales_properties m_properties;
      boost::optional<time_and_sales_window_model> m_model;
  };
}

#endif
