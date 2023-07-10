#ifndef SPIRE_DROP_DOWN_BOX_HPP
#define SPIRE_DROP_DOWN_BOX_HPP
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

      /** The type of function used to build a QWidget representing a value. */
      template<typename T = void>
      using ViewBuilder = ListView::ViewBuilder<T>;

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
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        ViewBuilder<> view_builder, QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox using default local models.
       * @param list The model of list of values to display.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list, ViewBuilder<T> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current, ViewBuilder<> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      DropDownBox(std::shared_ptr<AnyListModel> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder<> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current, ViewBuilder<T> view_builder,
        QWidget* parent = nullptr);

      /**
       * Constructs a DropDownBox.
       * @param list The model of list of values to display.
       * @param current The current value model.
       * @param selection The selection model.
       * @param view_builder The ViewBuilder to use.
       * @param parent The parent widget.
       */
      template<std::derived_from<AnyListModel> T>
      DropDownBox(std::shared_ptr<T> list,
        std::shared_ptr<CurrentModel> current,
        std::shared_ptr<SelectionModel> selection, ViewBuilder<T> view_builder,
        QWidget* parent = nullptr);

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
      mutable SubmitSignal m_submit_signal;
      ListView* m_list_view;
      TextBox* m_text_box;
      Button* m_button;
      DropDownList* m_drop_down_list;
      QTimer m_timer;
      bool m_is_read_only;
      boost::optional<int> m_submission;
      boost::optional<PressObserver> m_button_press_observer;
      bool m_is_modified;
      bool m_is_mouse_press_on_list;
      QPoint m_mouse_press_position;
      QPointer<QWidget> m_hovered_item;
      boost::signals2::scoped_connection m_submit_connection;
      boost::signals2::scoped_connection m_current_connection;

      int get_index_under_mouse(const QPoint& global_point) const;
      void hover_enter(const QMouseEvent& event);
      void hover_leave();
      void revert_current();
      void submit();
      void on_button_press_end(PressObserver::Reason reason);
      void on_current(const boost::optional<int>& current);
      void on_submit(const std::any& submission);
  };

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list, ViewBuilder<T> view_builder,
    QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
      [view_builder = std::move(view_builder)] (
          const std::shared_ptr<AnyListModel>& model, int index) {
        return view_builder(std::static_pointer_cast<T>(model), index);
      }, parent) {}

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current, ViewBuilder<T> view_builder,
    QWidget* parent)
    : DropDownBox(std::move(list), std::move(current),
        std::make_shared<ListSingleSelectionModel>(), std::move(view_builder),
        parent) {}

  template<std::derived_from<AnyListModel> T>
  DropDownBox::DropDownBox(std::shared_ptr<T> list,
    std::shared_ptr<CurrentModel> current,
    std::shared_ptr<SelectionModel> selection, ViewBuilder<T> view_builder,
    QWidget* parent)
    : DropDownBox(std::static_pointer_cast<AnyListModel>(list),
        std::move(current), std::move(selection),
        [view_builder = std::move(view_builder)] (
            const std::shared_ptr<AnyListModel>& model, int index) {
          return view_builder(std::static_pointer_cast<T>(model), index);
        }, parent) {}
}

#endif
