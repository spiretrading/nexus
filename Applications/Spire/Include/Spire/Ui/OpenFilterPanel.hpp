#ifndef SPIRE_OPEN_FILTER_PANEL_HPP
#define SPIRE_OPEN_FILTER_PANEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {
namespace Details {
  struct AbstractOpenInputBox {
    virtual void clear_current() = 0;

    virtual QWidget* get_widget() const = 0;

    virtual ~AbstractOpenInputBox() = default;
  };

  template<typename T>
  struct WrappingOpenInputBox : AbstractOpenInputBox {
    T m_input_box;

    explicit WrappingOpenInputBox(T box)
      : m_input_box(std::move(box)) {}

    void clear_current() override {
      using Type = std::decay_t<decltype(*m_input_box->get_current())>::Type;
      m_input_box->get_current()->set(Type());
    }

    QWidget* get_widget() const override {
      return m_input_box;
    }
  };

  struct OpenInputBox {
    std::unique_ptr<AbstractOpenInputBox> m_input_box;

    template<typename T>
    OpenInputBox(T box)
      : m_input_box(std::make_unique<WrappingOpenInputBox<T>>(std::move(box))) {}

    void clear_current() {
      m_input_box->clear_current();
    }

    QWidget* get_widget() const {
      return m_input_box->get_widget();
    }
  };
}

  /** Displays a FilterPanel over an open list of values.  */
  class OpenFilterPanel : public QWidget {
    public:

      /** Specifies whether the filtered values are included or excluded. */
      enum class FilterMode {

        /** The filtered values are included. */
        INCLUDE,

        /** The filtered values are excluded. */
        EXCLUDE
      };

      /** The OpenInputBox using to get input inside the OpenFilterPanel. */
      using OpenInputBox = typename Details::OpenInputBox;

      /** The type of model using to retrieve potential matches to a query. */
      using QueryModel = ComboBox::QueryModel;

      /**
       * The type of function used to handle the SubmitSignal of the ComboBox.
       * @param submission The submitted value.
       */
      using SubmitHandler = std::function<void(const std::any& submission)>;

      /**
       * The type of function used to build a particular input box.
       * @param query_model The model used to query matches by the ComboBox.
       * @param filtered_model The model contains a list of terms filtered out.
       * @param submit_handler The handler used to handle the SubmitSignal.
       * @return The QWidget that shall be a particular ComboBox.
       */
      using InputBoxBuilder = std::function<
        OpenInputBox (const std::shared_ptr<QueryModel>& query_model,
          const std::shared_ptr<AnyListModel>& filtered_model,
          const SubmitHandler& submit_handler)>;

      /**
       * Signals the list of filted values and the filter mode.
       * @param submission The list of values that the user wants to filter.
       * @param mode The filter mode.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<AnyListModel>& submission,
          FilterMode mode)>;

      /** The default InputBox builder which builds a ComboBox */
      static OpenInputBox default_input_box_builder(
        const std::shared_ptr<QueryModel>& query_model,
        const std::shared_ptr<AnyListModel>& filtered_model,
        const SubmitHandler& submit_handler);

      /**
       * Constructs an OpenFilterPanel using a defalut InputBoxBuilder.
       * @param query_model The model used to query matches.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
        QString title, QWidget& parent);

      /**
       * Constructs an OpenFilterPanel.
       * @param query_model The model used to query matches.
       * @param input_box_builder The InputBoxBuilder to use.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
        InputBoxBuilder input_box_builder, QString title, QWidget& parent);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      class FilterModeButtonGroup;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ArrayListModel<std::any>> m_filtered;
      std::unique_ptr<FilterModeButtonGroup> m_mode_button_group;
      std::unique_ptr<OpenInputBox> m_input_box;
      ScrollableListBox* m_scrollable_list_box;
      FilterPanel* m_filter_panel;
      boost::signals2::scoped_connection m_mode_current_connection;
      boost::signals2::scoped_connection m_filtered_connection;

      QWidget* make_item(const std::shared_ptr<AnyListModel>& model, int index);
      void on_input_box_submission(const std::any& submission);
      void on_list_view_current(const boost::optional<int>& current);
      void on_mode_current(FilterMode mode);
      void on_operation(const AnyListModel::Operation& operation);
      void on_reset();
  };
}

#endif
