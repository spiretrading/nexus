#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <QWidget>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Sets the spacing between list items. */
  using ListItemGap = BasicProperty<int, struct ListItemGapTag>;

  /**
   * Sets the gap between list items on overflow=WRAP in direction perpendicular
   * to list direction.
   */
  using ListOverflowGap = BasicProperty<int, struct ListOverflowGapTag>;

  /**
   * Specifies the keyboard navigation behavior when the first or last list
   * item is selected and the key for next or previous list item is pressed.
   */
  enum class EdgeNavigation {

    /** Selection stops at the current selection. */
    CONTAIN,

    /** Selection moves from the first item to last item and vice versa. */
    WRAP
  };

  /** Specifies how to layout items on overflow. */
  enum class Overflow {

    /** The list extends indefinitely. */
    NONE,

    /** List items wrap to fill the perpendicular space. */
    WRAP
  };

  /** Specifies the selection behavior for the ListView. */
  enum class SelectionMode {

    /** Items can not be selected. */
    NONE,

    /** The user can select a single item. */
    SINGLE
  };
}

  /**
   * Displays a list of values represented by ListItems stacked horizontally
   * or vertically.
   */
  class ListView : public QWidget {
    public:

      /**
       * The type of model representing the index of the current value.
       */
      using CurrentModel = ValueModel<boost::optional<std::any>>;

      /**
       * The type of model representing the index of the selected value.
       */
      using SelectionModel = ValueModel<boost::optional<std::any>>;

      /**
       * The type of function used to build a QWidget representing a value.
       * @param model The model containing the list values being displayed.
       * @param index The index of the specific value to be displayed.
       * @return The QWidget that shall be used to display the value in the
       *         <i>model</i> at the given <i>index</i>.
       */
      using ViewBuilder =
        std::function<QWidget* (const ArrayListModel& model, int index)>;

      /**
       * Signals that the current item was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /**
       * Constructs a ListView using default local models and a default view
       * builder.
       * @param list_model The model of values to display.
       * @param parent The parent widget.
       */
      explicit ListView(std::shared_ptr<ArrayListModel> list_model,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list_model The model of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<ArrayListModel> list_model,
        ViewBuilder view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param current_model The current value's model.
       * @param selection_model The selection value's model.
       * @param list_model The list model which holds a list of items.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<ArrayListModel> list_model,
        ViewBuilder view_builder, std::shared_ptr<CurrentModel> current_model,
        std::shared_ptr<SelectionModel> selection_model,
        QWidget* parent = nullptr);

      /** Returns the list of values displayed. */
      const std::shared_ptr<ArrayListModel>& get_list_model() const;
  
      /** Returns the current model. */
      const std::shared_ptr<CurrentModel>& get_current_model() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection_model() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

      QSize sizeHint() const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      struct ItemEntry {
        ListItem* m_item;
        int m_index;
      };
      struct BodyContainer;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ArrayListModel> m_list_model;
      ViewBuilder m_view_builder;
      std::shared_ptr<CurrentModel> m_current_model;
      std::shared_ptr<SelectionModel> m_selection_model;
      std::vector<std::unique_ptr<ItemEntry>> m_items;
      Box* m_box;
      BodyContainer* m_container;
      int m_item_gap;
      int m_overflow_gap;
      Qt::Orientation m_direction;
      Styles::EdgeNavigation m_edge_navigation;
      Styles::Overflow m_overflow;
      Styles::SelectionMode m_selection_mode;
      int m_current_index;
      QPoint m_navigation_point;

      void navigate_home();
      void navigate_end();
      void navigate_next();
      void navigate_previous();
      void navigate(
        int direction, int start, Styles::EdgeNavigation edge_navigation);
      void navigate_up();
      void navigate_down();
      void navigate_left();
      void navigate_right();
      void update_layout();
      void on_current(ItemEntry& item);
      void on_style();
  };
}

#endif
