#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <concepts>
#include <memory>
#include <unordered_set>
#include <vector>
#include <Beam/Threading/TaskRunner.hpp>
#include <boost/optional/optional.hpp>
#include <QSpacerItem>
#include <QTimer>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListCurrentController.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListSelectionController.hpp"
#include "Spire/Ui/ListViewItemBuilder.hpp"
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

  /** Specifies how to layout items on overflow. */
  enum class Overflow {

    /** The list extends indefinitely. */
    NONE,

    /** List items wrap to fill the perpendicular space. */
    WRAP
  };

  using EdgeNavigation = ListCurrentController::EdgeNavigation;
}

namespace Details {
  template<typename T>
  auto try_to_text(const T& value) {
    if constexpr(
        requires {{ to_text(value) } -> std::convertible_to<QString>;}) {
      return to_text(value);
    }
    return QString();
  }
}

  /**
   * Displays a list of values represented by ListItems stacked horizontally
   * or vertically.
   */
  class ListView : public QWidget {
    public:

      /** The type of model representing the index of the current value. */
      using CurrentModel = ListCurrentController::CurrentModel;

      /** The type of model representing the list of selected indicies. */
      using SelectionModel = ListSelectionController::SelectionModel;

      /** The type of function used to query the value. */
      using ToText = std::function<QString (const std::any&)>;

      /**
       * Signals that the current item was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /**
       * The default item builder which uses a label styled TextBox to display
       * the text representation of its value.
       */
      static QWidget* default_item_builder(
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
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to query items.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to query items.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param list The list model which holds a list of items.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param list The list model which holds a list of items.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to query items.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView.
       * @param list The list model which holds a list of items.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
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

      /**
       * Sets the size policy of ListItems along the direction they're laid out.
       */
      void set_direction_size_policy(QSizePolicy::Policy policy);

      /**
       * Sets the size policy of ListItems perpendicular to the direction
       * they're laid out.
       */
      void set_perpendicular_size_policy(QSizePolicy::Policy policy);

      /**
       * Sets the size policy of the ListItems along the direction they're laid
       * out as well as to its perpendicular direction.
       * @param direction_policy The size policy to set along the direction
       *        of the layout.
       * @param perpendicular_policy The size policy to set along the
       *        perpendicular direction of the layout.
       */
      void set_item_size_policy(QSizePolicy::Policy direction_policy,
        QSizePolicy::Policy perpendicular_policy);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      bool focusNextPrevChild(bool next) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;
      void moveEvent(QMoveEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      struct ItemEntry {
        ListItem m_item;
        int m_index;
        boost::optional<ClickObserver> m_click_observer;

        ItemEntry(int index);
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AnyListModel> m_list;
      boost::optional<int> m_focus_index;
      std::unordered_set<Qt::Key> m_keys;
      ListCurrentController m_current_controller;
      ListSelectionController m_selection_controller;
      ListViewItemBuilder<> m_item_builder;
      ToText m_to_text;
      std::vector<std::unique_ptr<ItemEntry>> m_items;
      std::vector<std::unique_ptr<ItemEntry>> m_pending_removals;
      ItemEntry* m_current_entry;
      Box* m_box;
      int m_top_index;
      int m_visible_count;
      QSizePolicy::Policy m_direction_policy;
      QSizePolicy::Policy m_perpendicular_policy;
      int m_item_gap;
      int m_overflow_gap;
      Qt::Orientation m_direction;
      Styles::Overflow m_overflow;
      QString m_query;
      QTimer m_query_timer;
      bool m_is_transaction;
      bool m_is_tab_focus_in;
      Beam::Threading::TaskRunner m_operation_queue;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_selection_connection;

      void append_query(const QString& query);
      void update_focus(boost::optional<int> current);
      ItemEntry& make_item_entry(int index);
      void add_item(int index);
      void pre_remove_item(int index);
      void remove_item(int index);
      void move_item(int source, int destination);
      void select_current();
      void update_layout();
      void update_parent();
      void initialize_visible_region();
      void update_visible_region();
      void on_item_click(ItemEntry& item);
      void on_list_operation(const AnyListModel::Operation& operation);
      void on_current(boost::optional<int> current);
      void on_selection(const ListModel<int>::Operation& operation);
      void on_item_submitted(ItemEntry& item);
      void on_style();
      void on_query_timer_expired();
  };

  /**
   * Selects an index in a ListView and sets that index to the current value.
   * @param list_view The ListView to navigate.
   * @param index The index within the <i>list_view</i> to navigate to.
   */
  void navigate_to_index(ListView& list_view, int index);

  /**
   * Selects a value in a ListView and sets it to the current value.
   * @param list_view The ListView to navigate.
   * @param value The value within the <i>list_view</i> to navigate to.
   */
  template<typename T>
  void navigate_to_value(ListView& list_view, const T& value) {
    auto list = std::dynamic_pointer_cast<ListModel<T>>(list_view.get_list());
    if(!list) {
      throw std::runtime_error("Invalid list provided for navigation.");
    }
    auto i = std::find(list->begin(), list->end(), value);
    if(i == list->end()) {
      return;
    }
    auto index = static_cast<int>(std::distance(list->begin(), i));
    navigate_to_index(list_view, index);
  }

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list),
        ListViewItemBuilder<>(std::move(item_builder)),
        [] (const std::any& value) {
          return Details::try_to_text(
            std::any_cast<const typename T::Type&>(value));
        }) {}

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list),
        std::move(selection), ListViewItemBuilder<>(std::move(item_builder)),
        [] (const std::any& value) {
          return Details::try_to_text(
            std::any_cast<const typename T::Type&>(value));
        }) {}

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list), std::move(current),
        std::move(selection), ListViewItemBuilder<>(std::move(item_builder)),
        [] (const std::any& value) {
          return Details::try_to_text(
            std::any_cast<const typename T::Type&>(value));
        }) {}
}

#endif
