#ifndef SPIRE_TIME_AND_SALES_WINDOW_HPP
#define SPIRE_TIME_AND_SALES_WINDOW_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/TableModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesProperties.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Window.hpp"

namespace Spire {
namespace Styles {

  /** Selects the row where the bbo indicator is unknown. */
  using UnknownRow = StateSelector<void, struct UnknownRowSelectorTag>;

  /** Selects the row where the bbo indicator is ABOVE_ASK. */
  using AboveAskRow = StateSelector<void, struct AboveAskRowSelectorTag>;

  /** Selects the row where the bbo indicator is AT_ASK. */
  using AtAskRow = StateSelector<void, struct AtAskRowSelectorTag>;

  /** Selects the row where the bbo indicator is INSIDE. */
  using InsideRow = StateSelector<void, struct InsideRowSelectorTag>;

  /** Selects the row where the bbo indicator is AT_BID. */
  using AtBidRow = StateSelector<void, struct AtBidRowSelectorTag>;

  /** Selects the row where the bbo indicator is BELOW_BID. */
  using BelowBidRow = StateSelector<void, struct BelowBidRowSelectorTag>;
}

  /** Display the time and sales window for a security. */
  class TimeAndSalesWindow : public Window {
    public:

      /**
       * The type of function used to build a TimeAndSalesModel based on
       * the security.
       * @param security The security that the window is representing.
       * @return A TimeAndSalesModel.
       */
      using ModelBuilder = std::function<std::shared_ptr<TimeAndSalesModel> (
        const Nexus::Security& security)>;

      /**
       * Constructs a TimeAndSalesWindow.
       * @param query_model The model used to query security.
       * @param properties The display properties of the window.
       * @param parent The parent widget.
       */
      TimeAndSalesWindow(std::shared_ptr<ComboBox::QueryModel> query_model,
        TimeAndSalesProperties properties, ModelBuilder model_builder,
        QWidget* parent = nullptr);

      /* Returns the time and sales model. */
      const std::shared_ptr<TimeAndSalesModel>& get_model() const;

      /* Returns the query model. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /* Returns the properties. */
      const TimeAndSalesProperties& get_properties() const;

      /* Returns the security that the window represents. */
      const std::shared_ptr<ValueModel<Nexus::Security>>& get_security() const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      TimeAndSalesProperties m_properties;
      ModelBuilder m_model_builder;
      TitleBar* m_title_bar;
      TimeAndSalesTableView* m_table_view;
      TransitionView* m_transition_view;
      SecurityView* m_security_view;
      ResponsiveLabel* m_responsive_title_label;
      ContextMenu* m_context_menu;

      int get_row_height() const;
      void make_context_menu();
      void update_export_menu_item();
      void on_current(const Nexus::Security& security);
      void on_table_operation(const TableModel::Operation& operation);
      void on_export();
  };
}

#endif
