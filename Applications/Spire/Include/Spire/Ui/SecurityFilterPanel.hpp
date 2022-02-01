#ifndef SPIRE_SECURITY_FILTER_PANEL_HPP
#define SPIRE_SECURITY_FILTER_PANEL_HPP
#include "Nexus/Definitions/Security.hpp"
#include "Spire/Spire/ListModel.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Displays an OpenFilterPanel over an open list of security values. */
  class SecurityFilterPanel : public QWidget {
    public:

      /** Specifies whether the matched values are included or excluded. */
      using Mode = OpenFilterPanel::Mode;

      /**
       * Signals the submission of a list of matched values and the filter mode.
       * @param matches The list of security values to match against.
       * @param mode The filter mode, whether to include or exclude
       *        <i>matches</i>.
       */
      using SubmitSignal = Signal<void (
        const std::shared_ptr<ListModel<Nexus::Security>>& matches, Mode mode)>;

      /**
       * Constructs a SecurityFilterPanel over an empty list of matches.
       * @param query_model The model used to query matches.
       * @param parent The parent widget.
       */
      SecurityFilterPanel(std::shared_ptr<ComboBox::QueryModel> query_model,
        QWidget& parent);

      /**
       * Constructs a SecurityFilterPanel.
       * @param query_model The model used to query matches.
       * @param matches The list of secuirty values to match against.
       * @param mode The filter mode.
       * @param parent The parent widget.
       */
      SecurityFilterPanel(std::shared_ptr<ComboBox::QueryModel> query_model,
        std::shared_ptr<ListModel<Nexus::Security>> matches,
        std::shared_ptr<ValueModel<Mode>> mode, QWidget& parent);

      /** Returns the model used to query matches. */
      const std::shared_ptr<ComboBox::QueryModel>& get_query_model() const;

      /** Returns the list of matches. */
      const std::shared_ptr<ListModel<Nexus::Security>>& get_matches() const;

      /** Returns the filter mode. */
      const std::shared_ptr<ValueModel<Mode>>& get_mode() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      OpenFilterPanel* m_panel;
      QWidget* m_filter_panel;
      std::shared_ptr<ComboBox::QueryModel> m_query_model;
      std::shared_ptr<ListModel<Nexus::Security>> m_matches;
  };
}

#endif
