#ifndef SPIRE_DROP_DOWN_BOX_HPP
#define SPIRE_DROP_DOWN_BOX_HPP
#include <functional>
#include <QPointer>
#include <QTimer>
#include <QWidget>
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/SingleSelectionModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Styles {

  /** Selects a widget displaying a pop-up, ie. the DropDownBox's list. */
  using PopUp = StateSelector<void, struct PopUpSelectorTag>;
}

  /**
   * Represents a widget which allows the user to choose one value from
   * a drop down list.
   */
  class DropDownBox : public QWidget {
    public:

      /** The type of model representing the index of the current value. */
      using CurrentModel = ListView::CurrentModel;

      /** The type of model representing the index of the selected value. */
      using SelectionModel = ListView::SelectionModel;

      /** The type of function used to display the value. */
      using ToText = std::function<QString (const std::any&)>;

      /**
       * Signals that the value was submitted.
       * @param submission The submitted value.
       */
      using SubmitSignal = Signal<void (const std::any& submission)>;

      /**
       * Constructs a DropDownBox using default local models and a default view
       * builder.
       * @param list The model of list of values to display.
       * @param parent The parent widget.
       */
      explicit DropDownBox(
        std::shared_ptr<AnyListModel> list, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T,
        std::invocable<const typename T::Type&> F>
      DropDownBox(std::shared_ptr<T> list,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        F&& to_text, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T,
        std::invocable<const typename T::Type&> F>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        F&& to_text, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<> item_builder, ToText to_text,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param item_builder The ListViewItemBuilder to use.
       * @param to_text The function used to display the current value.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T,
        std::invocable<const typename T::Type&> F>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection,
        ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
        F&& to_text, QWidget* parent = nullptr);

      /** Returns the model of list of values displayed. */
      const std::shared_ptr<AnyListModel>& get_list() const;

      /** Returns the current value model. */
      const std::shared_ptr<CurrentModel>& get_current() const;

      /** Returns the selection model. */
      const std::shared_ptr<SelectionModel>& get_selection() const;

      /** Returns <code>true</code> iff this DropDownBox is read-only. */
      bool is_read_only() const;

      /**
       * Sets the read-only state.
       * @param is_read_only True iff the DropDownBox should be read-only.
       */
      void set_read_only(bool is_read_only);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

    private:
      struct DropDownPanelWrapper {
        using DropDownPanel = std::variant<DropDownList*, EmptyState*>;
        DropDownPanel m_panel;

        void set(DropDownList& drop_down_list);
        void set(EmptyState& empty_state);
        DropDownList* get_drop_down_list() const;
        EmptyState* get_empty_state() const;
        QWidget* get_drop_down_window() const;
        bool is_visible() const;
        void destroy();
        void show();
      };
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AnyListModel> m_list;
      std::shared_ptr<CurrentModel> m_current;
      std::shared_ptr<SelectionModel> m_selection;
      ListViewItemBuilder<> m_item_builder;
      ToText m_to_text;
      TextBox* m_text_box;
      Button* m_button;
      QTimer m_timer;
      bool m_is_read_only;
      boost::optional<int> m_submission;
      boost::optional<PressObserver> m_button_press_observer;
      bool m_is_mouse_press_on_list;
      QPoint m_mouse_press_position;
      QPointer<QWidget> m_hovered_item;
      DropDownPanelWrapper m_drop_down_panel;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_current_connection;

      int get_index_under_mouse(const QPoint& global_point) const;
      void enter_hovered_item(const QMouseEvent& event);
      void leave_hovered_item();
      void revert_current();
      bool is_drop_down_panel_visible() const;
      void make_drop_down_empty_state();
      void make_drop_down_list();
      void make_drop_down_panel();
      void show_drop_down_panel();
      void hide_drop_down_panel();
      void submit();
      void on_button_press_end(PressObserver::Reason reason);
      void on_current(const boost::optional<int>& current);
      void on_submit(const std::any& submission);
  };

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        ListViewItemBuilder<>(std::move(item_builder)),
        [] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : DropDownBox(std::move(list), std::move(current),
        std::make_shared<ListSingleSelectionModel>(), std::move(item_builder),
        [] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}

  template<std::derived_from<AnyListModel> T,
    std::invocable<const typename T::Type&> F>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    F&& to_text, QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        ListViewItemBuilder<>(std::move(item_builder)),
        [to_text = std::forward<F>(to_text)] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}

  template<std::derived_from<AnyListModel> T,
    std::invocable<const typename T::Type&> F>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    F&& to_text, QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        std::move(current), ListViewItemBuilder<>(std::move(item_builder)),
        [to_text = std::forward<F>(to_text)] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        std::move(current), std::move(selection),
        ListViewItemBuilder<>(std::move(item_builder)),
        [] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}

  template<std::derived_from<AnyListModel> T,
    std::invocable<const typename T::Type&> F>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection,
    ListViewItemBuilder<ListModel<typename T::Type>> item_builder,
    F&& to_text, QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        std::move(current), std::move(selection),
        ListViewItemBuilder<>(std::move(item_builder)),
        [to_text = std::forward<F>(to_text)] (const std::any& value) {
          return to_text(std::any_cast<const typename T::Type&>(value));
        }, parent) {}
}

#endif
