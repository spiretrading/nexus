#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct ViewBuilder {
    using type =
      std::function<QWidget* (const std::shared_ptr<T>&, int)>;
  };

  template<>
  struct ViewBuilder<void> {
    using type =
      std::function<QWidget* (const std::shared_ptr<AnyListModel>&, int)>;
  };
}
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
      using CurrentModel = ValueModel<boost::optional<int>>;

      /**
       * The type of model representing the index of the selected value.
       */
      using SelectionModel = ValueModel<boost::optional<int>>;

      /**
       * The type of function used to build a QWidget representing a value.
       * @param list The list values being displayed.
       * @param index The index of the specific value to be displayed.
       * @return The QWidget that shall be used to display the value in the
       *         <i>list</i> at the given <i>index</i>.
       */
      template<typename T = void>
      using ViewBuilder = typename Details::ViewBuilder<T>::type;

      /**
       * Signals that the current item was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /**
       * The default view builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_view_builder(
        const std::shared_ptr<AnyListModel>& list, int index);

      /**
       * Constructs a ListView using default local models and a default view
       * builder.
       * @param list The model of values to display.
       * @param parent The parent widget.
       */
      explicit ListView(
        std::shared_ptr<AnyListModel> list, QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list, ViewBuilder<> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list, ViewBuilder<T> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param list The list model which holds a list of items.
       * @param current The current value model.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder<> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param list The list model which holds a list of items.
       * @param current The current value model.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder<T> view_builder,
        QWidget* parent = nullptr);

      /** Returns the list of values displayed. */
      const std::shared_ptr<AnyListModel>& get_list() const;

      /** Returns the current value model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

      /**
       * Returns the ListItem at a specified index, or <code>nullptr</code> iff
       * the index is out of range.
       */
      ListItem* get_list_item(int index);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

    private:
      struct ItemEntry {
        ListItem* m_item;
        int m_index;
        bool m_is_current;
        boost::signals2::scoped_connection m_connection;

        void set(bool is_current);
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AnyListModel> m_list;
      std::shared_ptr<CurrentModel> m_current;
      boost::optional<int> m_last_current;
      boost::optional<int> m_focus_index;
      std::shared_ptr<SelectionModel> m_selection;
      ViewBuilder<> m_view_builder;
      boost::optional<int> m_selected;
      std::vector<std::unique_ptr<ItemEntry>> m_items;
      Box* m_box;
      int m_item_gap;
      int m_overflow_gap;
      Qt::Orientation m_direction;
      Styles::EdgeNavigation m_edge_navigation;
      Styles::Overflow m_overflow;
      Styles::SelectionMode m_selection_mode;
      QRect m_navigation_box;
      QString m_query;
      QTimer* m_query_timer;
      Qt::FocusReason m_focus_reason;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_selection_connection;

      void append_query(const QString& query);
      void navigate_home();
      void navigate_end();
      void navigate_next();
      void navigate_previous();
      void navigate(
        int direction, int start, Styles::EdgeNavigation edge_navigation);
      void cross_next();
      void cross_previous();
      void cross(int direction);
      void set(boost::optional<int> current);
      void update_focus(boost::optional<int> current);
      void add_item(int index);
      void remove_item(int index);
      void move_item(int source, int destination);
      void update_layout();
      void on_list_operation(const AnyListModel::Operation& operation);
      void on_current(const boost::optional<int>& current);
      void on_selection(const boost::optional<int>& selected);
      void on_item_submitted(ItemEntry& item);
      void on_style();
      void on_query_timer_expired();
  };

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list, ViewBuilder<T> view_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list),
      [view_builder = std::move(view_builder)] (
          const std::shared_ptr<AnyListModel>& model, int index) {
        return view_builder(std::static_pointer_cast<T>(model), index);
      }, parent) {}

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder<T> view_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list), std::move(current),
        std::move(selection),
        [view_builder = std::move(view_builder)] (
            const std::shared_ptr<AnyListModel>& model, int index) {
          return view_builder(std::static_pointer_cast<T>(model), index);
        }, parent) {}
}

#endif
