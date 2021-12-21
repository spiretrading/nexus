#ifndef SPIRE_OPEN_FILTER_PANEL_HPP
#define SPIRE_OPEN_FILTER_PANEL_HPP
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/ListModel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

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

      /** The type of model using to retreive potential matches to a query. */
      using QueryModel = ComboBox::QueryModel;

      /**
       * The type of function used to handle the SubmitSignal of the ComboBox.
       * @param submission The submitted value.
       */
      using SubmitHandler = std::function<void(const std::any& submission)>;

      /**
       * The type of function used to build a particular ComboBox.
       * @param query_model The model used to query matches by the ComboBox.
       * @param submit_handler The handler used to handle the SubmitSignal.
       * @return The QWidget that shall be a particular ComboBox.
       */
      using ComboBoxBuilder = std::function<
        QWidget* (const std::shared_ptr<QueryModel>& query_model,
          const SubmitHandler& submit_handler)>;

      /**
       * Signals the list of filted values and the filter mode.
       * @param submission The list of values that the user wants to filter.
       * @param mode The filter mode.
       */
      using SubmitSignal =
        Signal<void (const std::shared_ptr<ListModel>& submission,
          FilterMode mode)>;

      /** The default ComboBox builder which uses an usual ComboBox */
      static QWidget* default_combo_box_builder(
        const std::shared_ptr<QueryModel>& query_model,
        const SubmitHandler& submit_handler);

      /**
       * Constructs an OpenFilterPanel using a defalut ComboBoxBuilder.
       * @param query_model The model used to query matches.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
        QString title, QWidget& parent);

      /**
       * Constructs an OpenFilterPanel.
       * @param query_model The model used to query matches.
       * @param combo_box_builder The ComboBoxBuilder to use.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
        ComboBoxBuilder combo_box_builder, QString title, QWidget& parent);

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      class FilterModeButtonGroup;
      //struct OpenFilterQueryModel;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ArrayListModel> m_filtered;
      //std::shared_ptr<OpenFilterQueryModel> m_query_model;
      std::unique_ptr<FilterModeButtonGroup> m_mode_button_group;
      QWidget* m_combo_box;
      ScrollableListBox* m_scrollable_list_box;
      FilterPanel* m_filter_panel;
      boost::signals2::scoped_connection m_filtered_connection;

      QWidget* make_item(const std::shared_ptr<ListModel>& model, int index);
      void on_list_view_current(const boost::optional<int>& current);
      void on_mode_current(FilterMode mode);
      void on_operation(const ListModel::Operation& operation);
      void on_reset();
  };
}

#endif
