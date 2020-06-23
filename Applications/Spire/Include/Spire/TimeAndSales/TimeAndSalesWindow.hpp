#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/optional.hpp>
#include <QLabel>
#include <QMenu>
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/TimeAndSales/TimeAndSales.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindowModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays a time and sales window.
  class TimeAndSalesWindow : public Window {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param s The security to display.
      */
      using ChangeSecuritySignal = Signal<void (const Nexus::Security& s)>;

      //! Signals that the recent colors have changed.
      /*!
        \param recent_colors The updated recent colors.
      */
      using RecentColorsSignal =
        Signal<void (const RecentColors& recent_colors)>;

      //! Constructs a time and sales window.
      /*!
        \param properties The display properties.
        \param recent_colors The recent colors for the properties dialog.
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      TimeAndSalesWindow(const TimeAndSalesProperties& properties,
        const RecentColors& recent_colors,
        Beam::Ref<SecurityInputModel> input_model, QWidget* parent = nullptr);

      //! Sets the model to display.
      void set_model(std::shared_ptr<TimeAndSalesModel> model);

      //! Returns the display properties.
      const TimeAndSalesProperties& get_properties() const;

      //! Sets the display properties.
      void set_properties(const TimeAndSalesProperties& properties);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_change_security_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

      //! Connects a slot to the recent colors signal.
      boost::signals2::connection connect_recent_colors_signal(
        const RecentColorsSignal::slot_type& slot) const;

    protected:
      void contextMenuEvent(QContextMenuEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      TimeAndSalesProperties m_properties;
      mutable RecentColorsSignal m_recent_colors_signal;
      RecentColors m_recent_colors;
      boost::optional<TimeAndSalesWindowModel> m_model;
      boost::signals2::scoped_connection m_volume_connection;
      SecurityWidget* m_security_widget;
      QWidget* m_body;
      TimeAndSalesTableView* m_table;
      QLabel* m_volume_label;
      CustomVariantItemDelegate* m_item_delegate;

      void export_table();
      void show_properties_dialog();
      void on_recent_colors_changed(const RecentColors& recent_colors);
      void on_volume(const Nexus::Quantity& volume);
  };
}

#endif
