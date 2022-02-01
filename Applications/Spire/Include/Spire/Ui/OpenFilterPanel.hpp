#ifndef SPIRE_OPEN_FILTER_PANEL_HPP
#define SPIRE_OPEN_FILTER_PANEL_HPP
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/ComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays a FilterPanel over an open list of matched values. */
  class OpenFilterPanel : public QWidget {
    public:

      /** Specifies whether the matched values are included or excluded. */
      enum class Mode {

        /** The matched values are included. */
        INCLUDE,

        /** The matched values are excluded. */
        EXCLUDE
      };

      /**
       * The type of function used to build the input box submitting values to
       * match against.
       * @param matches The list of existing matches. These values should not be
       *        resubmitted by the input box.
       * @return A new AnyInputBox to be displayed.
       */
      using InputBoxBuilder =
        std::function<AnyInputBox* (std::shared_ptr<AnyListModel> matches)>;

      /**
       * Signals the submission of a list of matched values and the filter mode.
       * @param matches The list of values to match against.
       * @param mode The filter mode, whether to include or exclude
       *        <i>matches</i>.
       */
      using SubmitSignal = Signal<void (
        const std::shared_ptr<AnyListModel>& matches, Mode mode)>;

      /**
       * Constructs an OpenFilterPanel over an empty list of matches using a
       * ComboBox as its input box.
       * @param query_model The model used by the ComboBox.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(std::shared_ptr<ComboBox::QueryModel> query_model,
        QString title, QWidget& parent);

      /**
       * Constructs an OpenFilterPanel over an empty list of matches.
       * @param input_box_builder The InputBoxBuilder to use.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(InputBoxBuilder input_box_builder, QString title,
        QWidget& parent);

      /**
       * Constructs an OpenFilterPanel.
       * @param input_box_builder The InputBoxBuilder to use.
       * @param matches The list of values to match against.
       * @param mode The filter mode.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanel(InputBoxBuilder input_box_builder,
        std::shared_ptr<AnyListModel> matches,
        std::shared_ptr<ValueModel<Mode>> mode, QString title, QWidget& parent);

      /** Returns the list of matches. */
      const std::shared_ptr<AnyListModel>& get_matches() const;

      /** Returns the filter mode. */
      const std::shared_ptr<ValueModel<Mode>>& get_mode() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      class FilterModeButtonGroup;
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<AnyListModel> m_matches;
      std::shared_ptr<ValueModel<Mode>> m_mode;
      std::unique_ptr<FilterModeButtonGroup> m_mode_button_group;
      AnyInputBox* m_input_box;
      ListView* m_list_view;
      FilterPanel* m_filter_panel;
      boost::signals2::scoped_connection m_mode_connection;
      boost::signals2::scoped_connection m_matches_connection;

      QWidget* make_item(const std::shared_ptr<AnyListModel>& model, int index);
      void on_input_box_submission(const AnyRef& submission);
      void on_mode_current(Mode mode);
      void on_matches_operation(const AnyListModel::Operation& operation);
      void on_reset();
      void submit();
  };
}

#endif
