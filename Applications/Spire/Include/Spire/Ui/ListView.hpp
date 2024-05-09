#ifndef SPIRE_LIST_VIEW_HPP
#define SPIRE_LIST_VIEW_HPP
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_set>
#include <QSpacerItem>
#include <QTimer>
#include <QWidget>
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Spire/Spire.hpp"
#include "Spire/Styles/BasicProperty.hpp"
#include "Spire/Ui/ClickObserver.hpp"
#include "Spire/Ui/ListCurrentController.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListSelectionController.hpp"
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

  template<typename T, typename M>
  concept ListViewBuilderConcept = requires(T builder) {
    { builder.mount(std::shared_ptr<M>(), int()) };
    { builder.unmount(static_cast<QWidget*>(nullptr), int()) };
  };

  /**
   * Builder class used to build QWidget's used by a ListView to represent the
   * elements of a ListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T = AnyListModel>
  class ListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      /**
       * Constructs a ListViewBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param view_builder The function to call for every element of the list.
       */
      ListViewBuilder(std::invocable<const std::shared_ptr<ListModel>&, int>
        auto view_builder);

      /**
       * Constructs a ListViewBuilder that calls a function to build a new
       * QWidget for every element, and calls delete to unmount widgets.
       * @param view_builder The function to call for every element of the list.
       */
      template<typename M>
      ListViewBuilder(
          std::invocable<const std::shared_ptr<Spire::ListModel<M>>&, int>
            auto view_builder) requires std::is_same<ListModel, AnyListModel>
        : ListViewBuilder([view_builder = std::move(view_builder)] (
              const std::shared_ptr<ListModel>& model, int index) {
            return view_builder(
              std::static_pointer_cast<Spire::ListModel<M>>(model), index);
          }) {}

      /** Constructs a ListViewBuilder. */
      template<ListViewBuilderConcept<ListModel> B>
      ListViewBuilder(B&& view_builder)
        : m_builder(std::make_shared<WrappedListViewBuilder<
            std::remove_reference_t<B>>>(std::forward<B>(view_builder))) {}

      /** Constructs a ListViewBuilder. */
      template<typename M>
      ListViewBuilder(ListViewBuilder<M> view_builder) requires
        std::is_same_v<ListModel, AnyListModel> &&
        !std::is_same_v<M, AnyListModel>;

      /**
       * Returns the QWidget to represent a list element.
       * @param model The list model containing the element to represent.
       * @param index The index of the element to represent.
       * @return The QWidget to use to represent the element.
       */
      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      /**
       * Releases a previously mounted QWidget.
       * @param widget The QWidget to release.
       * @param index The index of the element the <i>widget</i> represented.
       */
      void unmount(QWidget* widget, int index);

    private:
      struct VirtualListViewBuilder {
        virtual ~VirtualListViewBuilder() = default;
        virtual QWidget* mount(
          const std::shared_ptr<ListModel>& model, int index) = 0;
        virtual void unmount(QWidget* widget, int index) = 0;
      };
      template<typename B>
      struct WrappedListViewBuilder final : VirtualListViewBuilder {
        using Builder = B;
        Beam::GetOptionalLocalPtr<Builder> m_builder;

        template<typename... Args>
        WrappedListViewBuilder(Args&&... args);
        QWidget* mount(const std::shared_ptr<ListModel>& model, int index)
          override;
        void unmount(QWidget* widget, int index) override;
      };
      std::shared_ptr<VirtualListViewBuilder> m_builder;
  };

  /**
   * Implements a ListViewBuilder that mounts using an std::function and calls
   * delete to unmount.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class FunctionListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = T;

      /**
       * Constructs a FunctionListViewBuilder.
       * @param view_builder The function used to mount QWidgets.
       */
      FunctionListViewBuilder(
        std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
          view_builder);

      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      std::function<QWidget* (const std::shared_ptr<ListModel>&, int)>
        m_view_builder;
  };

  /**
   * Implements a ListViewBuilder that converts from a ListModel<T> to an
   * AnyListModel.
   * @param <T> The type of ListModel to mount QWidget's for.
   */
  template<typename T>
  class AnyListViewBuilder {
    public:

      /** The type of ListModel to mount QWidget's for. */
      using ListModel = AnyListModel;

      /**
       * Constructs an AnyListViewBuilder.
       * @param view_builder The ListViewBuilder to convert.
       */
      AnyListViewBuilder(ListViewBuilder<T> view_builder);

      QWidget* mount(const std::shared_ptr<ListModel>& model, int index);

      void unmount(QWidget* widget, int index);

    private:
      ListViewBuilder<T> m_view_builder;
  };

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
       * @param view_builder The ListViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        ListViewBuilder<> view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      ListView(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<SelectionModel> selection,
        ListViewBuilder<> view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        ListViewBuilder<ListModel<typename T::Type>> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a ListView using default local models.
       * @param list The model of values to display.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      ListView(std::shared_ptr<T> list,
        std::shared_ptr<SelectionModel> selection,
        ListViewBuilder<ListModel<typename T::Type>> view_builder,
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
        std::shared_ptr<SelectionModel> selection,
        ListViewBuilder<> view_builder, QWidget* parent = nullptr);

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
        std::shared_ptr<SelectionModel> selection,
        ListViewBuilder<ListModel<typename T::Type>> view_builder,
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
      ListViewBuilder<> m_view_builder;
      std::vector<std::unique_ptr<ItemEntry>> m_items;
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
      int m_initialize_count;
      bool m_is_transaction;
      boost::signals2::scoped_connection m_style_connection;
      boost::signals2::scoped_connection m_list_connection;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_selection_connection;

      void append_query(const QString& query);
      void update_focus(boost::optional<int> current);
      void make_item_entry(int index);
      void add_item(int index);
      void remove_item(int index);
      void move_item(int source, int destination);
      void update_layout();
      void update_parent();
      void initialize_visible_region();
      void update_visible_region();
      void on_item_click(ItemEntry& item);
      void on_list_operation(const AnyListModel::Operation& operation);
      void on_current(
        boost::optional<int> previous, boost::optional<int> current);
      void on_selection(const ListModel<int>::Operation& operation);
      void on_item_submitted(ItemEntry& item);
      void on_style();
      void on_query_timer_expired();
  };

  template<typename T>
  ListViewBuilder<T>::ListViewBuilder(std::invocable<
    const std::shared_ptr<ListModel>&, int> auto view_builder)
    : ListViewBuilder(FunctionListViewBuilder<ListModel>(
        std::move(view_builder))) {}

  template<typename T>
  template<typename M>
  ListViewBuilder<T>::ListViewBuilder(ListViewBuilder<M> view_builder) requires
    std::is_same_v<ListModel, AnyListModel> &&
    !std::is_same_v<M, AnyListModel>
    : ListViewBuilder(AnyListViewBuilder(std::move(view_builder))) {}

  template<typename T>
  QWidget* ListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_builder->mount(model, index);
  }

  template<typename T>
  void ListViewBuilder<T>::unmount(QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  template<typename B>
  template<typename... Args>
  ListViewBuilder<T>::WrappedListViewBuilder<B>::WrappedListViewBuilder(
    Args&&... args)
    : m_builder(std::forward<Args>(args)...) {}

  template<typename T>
  template<typename B>
  QWidget* ListViewBuilder<T>::WrappedListViewBuilder<B>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_builder->mount(model, index);
  }

  template<typename T>
  template<typename B>
  void ListViewBuilder<T>::WrappedListViewBuilder<B>::unmount(
      QWidget* widget, int index) {
    m_builder->unmount(widget, index);
  }

  template<typename T>
  FunctionListViewBuilder<T>::FunctionListViewBuilder(std::function<
    QWidget* (const std::shared_ptr<ListModel>&, int)> view_builder)
    : m_view_builder(std::move(view_builder)) {}

  template<typename T>
  QWidget* FunctionListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_view_builder(model, index);
  }

  template<typename T>
  void FunctionListViewBuilder<T>::unmount(QWidget* widget, int index) {
    delete widget;
  }

  template<typename T>
  AnyListViewBuilder<T>::AnyListViewBuilder(ListViewBuilder<T> view_builder)
    : m_view_builder(std::move(view_builder)) {}

  template<typename T>
  QWidget* AnyListViewBuilder<T>::mount(
      const std::shared_ptr<ListModel>& model, int index) {
    return m_view_builder.mount(std::static_pointer_cast<T>(model), index);
  }

  template<typename T>
  void AnyListViewBuilder<T>::unmount(QWidget* widget, int index) {
    m_view_builder.unmount(widget, index);
  }

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    ListViewBuilder<ListModel<typename T::Type>> view_builder,
    QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list),
        ListViewBuilder<>(std::move(view_builder))) {}

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    std::shared_ptr<SelectionModel> selection,
    ListViewBuilder<ListModel<typename T::Type>> view_builder, QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list),
        std::move(selection), ListViewBuilder<>(std::move(view_builder))) {}

  template<std::derived_from<AnyListModel> T>
  ListView::ListView(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewBuilder<ListModel<typename T::Type>> view_builder, QWidget* parent)
    : ListView(std::static_pointer_cast<AnyListModel>(list), std::move(current),
        std::move(selection), ListViewBuilder<>(std::move(view_builder))) {}
}

#endif
