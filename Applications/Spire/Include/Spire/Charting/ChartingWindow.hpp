#ifndef SPIRE_CHARTING_WINDOW_HPP
#define SPIRE_CHARTING_WINDOW_HPP
#include <memory>
#include <Beam/Pointers/Ref.hpp>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include "Spire/Charting/Charting.hpp"
#include "Spire/Charting/TrendLine.hpp"
#include "Nexus/Definitions/Security.hpp"
#include "Spire/SecurityInput/SecurityInput.hpp"
#include "Spire/Ui/RecentColors.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"
#include "Spire/Ui/Ui.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {

  //! Displays a financial chart.
  class ChartingWindow : public Window {
    public:

      //! Signals a request to change the displayed security.
      /*!
        \param security The security to display.
      */
      using ChangeSecuritySignal =
        Signal<void (const Nexus::Security& security)>;

      //! Constructs a charting window.
      /*!
        \param input_model The SecurityInputModel to use for autocomplete.
        \param parent The parent widget.
      */
      explicit ChartingWindow(Beam::Ref<SecurityInputModel> input_model,
        QWidget* parent = nullptr);

      //! Sets the chart and technicals models to display.
      /*!
        \param chart_model The chart model data source.
        \param technicals_model The technicals model data source.
      */
      void set_models(std::shared_ptr<ChartModel> chart_model,
        std::shared_ptr<TechnicalsModel> technicals_model);

      //! Connects a slot to the change security signal.
      boost::signals2::connection connect_security_change_signal(
        const ChangeSecuritySignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* object, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      std::shared_ptr<ChartModel> m_model;
      std::shared_ptr<TechnicalsModel> m_technicals_model;
      SecurityWidget* m_security_widget;
      QWidget* m_button_header_widget;
      QLineEdit* m_period_line_edit;
      StaticDropDownMenu* m_period_dropdown;
      ToggleButton* m_lock_grid_button;
      ToggleButton* m_auto_scale_button;
      ToggleButton* m_draw_line_button;
      bool m_is_chart_auto_scaled;
      TrendLineEditor* m_trend_line_editor_widget;
      QWidget* m_security_widget_container;
      ChartingTechnicalsPanel* m_technicals_panel;
      ChartView* m_chart;
      bool m_is_mouse_dragging;
      QPoint m_last_chart_mouse_pos;

      void on_auto_scale_button_click();
      void on_draw_line_button_click();
      void on_period_line_edit_changed();
      void on_security_change(const Nexus::Security& security);
      void on_trend_line_color_selected();
      void on_trend_line_style_selected();
  };
}

#endif
