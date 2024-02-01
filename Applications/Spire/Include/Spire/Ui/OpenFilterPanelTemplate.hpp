#ifndef SPIRE_OPEN_FILTER_PANEL_TEMPLATE_HPP
#define SPIRE_OPEN_FILTER_PANEL_TEMPLATE_HPP
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /**
   * Displays an OpenFilterPanel over an open list of matched values.
   * @param <T> The type of matched values.
   */
  template<typename T>
  class OpenFilterPanelTemplate : public QWidget {
    public:

      /** The type of matched values. */
      using Type = T;

      /** Specifies whether the matched values are included or excluded. */
      using Mode = OpenFilterPanel::Mode;

      /**
       * The type of function used to build the input box submitting values to
       * match against.
       */
      using InputBoxBuilder = OpenFilterPanel::InputBoxBuilder;

      /**
       * Signals the submission of a list of matched values and the filter mode.
       * @param matches The list of values to match against.
       * @param mode The filter mode, whether to include or exclude
       *        <i>matches</i>.
       */
      using SubmitSignal = Signal<void (
        const std::shared_ptr<ListModel<Type>>& matches, Mode mode)>;

      /**
       * Constructs an OpenFilterPanelTemplate over an empty list of matches.
       * @param input_box_builder The InputBoxBuilder to use.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanelTemplate(InputBoxBuilder input_box_builder, QString title,
        QWidget& parent);

      /**
       * Constructs an OpenFilterPanelTemplate.
       * @param input_box_builder The InputBoxBuilder to use.
       * @param matches The list of values to match against.
       * @param mode The filter mode.
       * @param title The title of the panel.
       * @param parent The parent widget.
       */
      OpenFilterPanelTemplate(InputBoxBuilder input_box_builder,
        std::shared_ptr<ListModel<Type>> matches,
        std::shared_ptr<ValueModel<Mode>> mode, QString title, QWidget& parent);

      /** Returns the list of matches. */
      const std::shared_ptr<ListModel<Type>>& get_matches() const;

      /** Returns the filter mode. */
      const std::shared_ptr<ValueModel<Mode>>& get_mode() const;

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const typename SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      bool event(QEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      std::shared_ptr<ListModel<Type>> m_matches;
      OpenFilterPanel* m_panel;
  };

  template<typename T>
  OpenFilterPanelTemplate<T>::OpenFilterPanelTemplate(
    InputBoxBuilder input_box_builder, QString title, QWidget& parent)
    : OpenFilterPanelTemplate(input_box_builder,
        std::make_shared<ArrayListModel<Type>>(),
        std::make_shared<LocalValueModel<Mode>>(Mode::INCLUDE),
        title, parent) {}

  template<typename T>
  OpenFilterPanelTemplate<T>::OpenFilterPanelTemplate(
      InputBoxBuilder input_box_builder,
      std::shared_ptr<ListModel<Type>> matches,
      std::shared_ptr<ValueModel<Mode>> mode, QString title, QWidget& parent)
      : QWidget(&parent),
        m_matches(std::move(matches)),
        m_panel(new OpenFilterPanel(std::move(input_box_builder), m_matches,
          std::move(mode), title, parent)) {
    m_panel->connect_submit_signal([=] (const auto& matches, auto mode) {
      m_submit_signal(m_matches, mode);
    });
    m_panel->window()->installEventFilter(this);
    hide();
  }

  template<typename T>
  const std::shared_ptr<ListModel<typename OpenFilterPanelTemplate<T>::Type>>&
      OpenFilterPanelTemplate<T>::get_matches() const {
    return m_matches;
  }

  template<typename T>
  const std::shared_ptr<ValueModel<typename OpenFilterPanelTemplate<T>::Mode>>&
      OpenFilterPanelTemplate<T>::get_mode() const {
    return m_panel->get_mode();
  }

  template<typename T>
  boost::signals2::connection OpenFilterPanelTemplate<T>::connect_submit_signal(
      const typename SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  bool OpenFilterPanelTemplate<T>::eventFilter(QObject* watched,
      QEvent* event) {
    if(event->type() == QEvent::Close) {
      m_panel->hide();
      hide();
    }
    return QWidget::eventFilter(watched, event);
  }

  template<typename T>
  bool OpenFilterPanelTemplate<T>::event(QEvent* event) {
    if(event->type() == QEvent::Show) {
      m_panel->show();
    } else if(event->type() == QEvent::Hide) {
      m_panel->hide();
    }
    return QWidget::event(event);
  }
}

#endif
