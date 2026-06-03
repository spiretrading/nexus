#ifndef SPIRE_OPEN_FILTER_PANEL_HPP
#define SPIRE_OPEN_FILTER_PANEL_HPP
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/Ui.hpp"

namespace Spire {

  /** Provides the operations an OpenFilterPanel needs from its input widget. */
  template<typename T>
  struct OpenFilterPanelAdaptor;

  /** An OpenFilterPanelAdaptor specialized for a TagComboBox. */
  template<typename T>
  struct OpenFilterPanelAdaptor<TagComboBox<T>> {

    /** The type of the input's current value. */
    using Type = std::shared_ptr<ListModel<T>>;

    /** Returns whether the input contains no values. */
    static bool is_empty(TagComboBox<T>& combo_box);

    /** Removes all values from the input. */
    static void clear(TagComboBox<T>& combo_box);

    /** Returns the current value of the input. */
    static Type get_current(TagComboBox<T>& combo_box);

    /** Connects a slot called when the input's current value changes. */
    static boost::signals2::connection connect_current(
      TagComboBox<T>& combo_box, const std::function<void()>& slot);
  };

  /** Displays a FilterPanel over an open list of values. */
  template<typename T>
  class OpenFilterPanel : public QWidget {
    public:

      /** The type of the tag list box. */
      using TagListBox = T;

      /** The type of the submission produced by the tag list box. */
      using SubmissionType = typename OpenFilterPanelAdaptor<T>::Type;

      /** Specifies whether the filtered values are included or excluded. */
      enum class Mode {

        /** The filtered values are included. */
        INCLUDE,

        /** The filtered values are excluded. */
        EXCLUDE
      };

      /**
       * Signals the submission of the filtered values and the filter mode.
       * @param filtered The list of values selected by the user.
       * @param mode Whether the filtered values are included or excluded.
       */
      using SubmitSignal =
        Signal<void (const SubmissionType& filtered, Mode mode)>;

      /**
       * Constructs a OpenFilterPanel.
       * @param tag_list_box The widget that displays the list of tags.
       * @param parent The parent widget.
       */
      explicit OpenFilterPanel(TagListBox& tag_list_box,
        QWidget* parent = nullptr);

      /** Returns the tag list box. */
      const TagListBox& get_tag_list_box() const;

      /** Returns the tag list box. */
      TagListBox& get_tag_list_box();

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      TagListBox* m_tag_list_box;
      std::shared_ptr<AssociativeValueModel<Mode>> m_mode;
      QWidget* m_body;
      QWidget* m_button_container;
      bool m_is_first_show;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_mode_connection;

      QString display_text(Mode mode) const;
      CheckBox* make_mode_check_box(Mode mode) const;
      QLayout* make_mode_section_layout(Mode mode) const;
      QWidget* find_next_focusable_widget() const;
      void submit();
      void on_current();
      void on_mode(Mode mode);
      void on_reset();
  };

  template<typename T>
  bool OpenFilterPanelAdaptor<TagComboBox<T>>::is_empty(
      TagComboBox<T>& combo_box) {
    return combo_box.get_current()->get_size() == 0;
  }

  template<typename T>
  void OpenFilterPanelAdaptor<TagComboBox<T>>::clear(
      TagComboBox<T>& combo_box) {
    Spire::clear(*combo_box.get_current());
  }

  template<typename T>
  typename OpenFilterPanelAdaptor<TagComboBox<T>>::Type
      OpenFilterPanelAdaptor<TagComboBox<T>>::get_current(
        TagComboBox<T>& combo_box) {
    return combo_box.get_current();
  }

  template<typename T>
  boost::signals2::connection
      OpenFilterPanelAdaptor<TagComboBox<T>>::connect_current(
        TagComboBox<T>& combo_box, const std::function<void()>& slot) {
    return combo_box.get_current()->connect_operation_signal(
      [=] (const auto& operation) {
        visit(operation,
          [&] (const ListModel<T>::AddOperation&) {
            slot();
          },
          [&] (const ListModel<T>::RemoveOperation&) {
            slot();
          });
      });
  }

  template<typename T>
  OpenFilterPanel<T>::OpenFilterPanel(TagListBox& tag_list_box,
      QWidget* parent)
      : QWidget(parent),
        m_tag_list_box(&tag_list_box),
        m_mode(std::make_shared<AssociativeValueModel<Mode>>(Mode::INCLUDE)),
        m_is_first_show(true) {
    m_tag_list_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_tag_list_box->setMinimumSize(scale(160, 26));
    m_tag_list_box->setMaximumHeight(scale_height(63));
    m_button_container = new QWidget();
    m_button_container->setSizePolicy(QSizePolicy::Fixed,
      QSizePolicy::Expanding);
    auto container_layout = make_hbox_layout(m_button_container);
    container_layout->setSpacing(scale_width(18));
    container_layout->addLayout(make_mode_section_layout(Mode::INCLUDE));
    container_layout->addLayout(make_mode_section_layout(Mode::EXCLUDE));
    m_body = new QWidget();
    m_body->installEventFilter(this);
    auto body_layout = make_hbox_layout(m_body);
    body_layout->setSpacing(scale_width(18));
    body_layout->addWidget(m_button_container);
    body_layout->addWidget(m_tag_list_box);
    auto panel = new FilterPanel(*m_body);
    enclose(*this, *panel);
    Styles::proxy_style(*this, *panel);
    setFocusProxy(m_tag_list_box);
    panel->connect_reset_signal(
      std::bind_front(&OpenFilterPanel<TagListBox>::on_reset, this));
    m_current_connection =
      OpenFilterPanelAdaptor<TagListBox>::connect_current(
        *m_tag_list_box,
        std::bind_front(&OpenFilterPanel<TagListBox>::on_current, this));
    m_mode_connection = m_mode->connect_update_signal(
      std::bind_front(&OpenFilterPanel<TagListBox>::on_mode, this));
  }

  template<typename T>
  const OpenFilterPanel<T>::TagListBox&
      OpenFilterPanel<T>::get_tag_list_box() const {
    return *m_tag_list_box;
  }

  template<typename T>
  OpenFilterPanel<T>::TagListBox& OpenFilterPanel<T>::get_tag_list_box() {
    return *m_tag_list_box;
  }

  template<typename T>
  boost::signals2::connection OpenFilterPanel<T>::connect_submit_signal(
      const SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  bool OpenFilterPanel<T>::eventFilter(QObject* watched, QEvent* event) {
    if(watched == m_body && event->type() == QEvent::Resize) {
      auto available_width =
        m_body->width() - m_button_container->width() - scale_width(18);
      m_tag_list_box->setMaximumWidth(std::max(0,
        std::max(m_tag_list_box->minimumWidth(), available_width)));
    }
    return QWidget::eventFilter(watched, event);
  }

  template<typename T>
  void OpenFilterPanel<T>::showEvent(QShowEvent* event) {
    if(m_is_first_show) {
      if(auto next = find_next_focusable_widget()) {
        setTabOrder(find_focus_proxy(*m_tag_list_box), next);
      }
      m_is_first_show = false;
    }
    QWidget::showEvent(event);
  }

  template<typename T>
  QString OpenFilterPanel<T>::display_text(Mode mode) const {
    if(mode == Mode::INCLUDE) {
      static const auto value = QObject::tr("Include");
      return value;
    }
    static const auto value = QObject::tr("Exclude");
    return value;
  }

  template<typename T>
  CheckBox* OpenFilterPanel<T>::make_mode_check_box(Mode mode) const {
    auto button = make_radio_button();
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->set_label(display_text(mode));
    button->get_current()->set(m_mode->get_association(mode)->get());
    button->get_current()->connect_update_signal([=] (auto value) {
      if(m_mode->get() == mode && !value) {
        button->get_current()->set(true);
      } else if(value) {
        m_mode->set(mode);
      }
    });
    m_mode->get_association(mode)->connect_update_signal(
      [=] (bool value) {
        button->get_current()->set(value);
      });
    return button;
  }

  template<typename T>
  QLayout* OpenFilterPanel<T>::make_mode_section_layout(Mode mode) const {
    auto layout = make_vbox_layout();
    layout->addSpacing(scale_height(5));
    layout->addWidget(make_mode_check_box(mode));
    layout->addStretch(1);
    return layout;
  }

  template<typename T>
  QWidget* OpenFilterPanel<T>::find_next_focusable_widget() const {
    auto next = m_tag_list_box->nextInFocusChain();
    while(next != m_tag_list_box && isAncestorOf(next)) {
      if(!m_body->isAncestorOf(next) && next->focusPolicy() & Qt::TabFocus) {
        return next;
      }
      next = next->nextInFocusChain();
    }
    return nullptr;
  }

  template<typename T>
  void OpenFilterPanel<T>::submit() {
    auto mode = [&] {
      if(OpenFilterPanelAdaptor<TagListBox>::is_empty(*m_tag_list_box)) {
        return Mode::EXCLUDE;
      }
      return m_mode->get();
    }();
    m_submit_signal(
      OpenFilterPanelAdaptor<TagListBox>::get_current(*m_tag_list_box),
      mode);
  }

  template<typename T>
  void OpenFilterPanel<T>::on_current() {
    submit();
  }

  template<typename T>
  void OpenFilterPanel<T>::on_mode(Mode mode) {
    submit();
  }

  template<typename T>
  void OpenFilterPanel<T>::on_reset() {
    auto current_blocker =
      boost::signals2::shared_connection_block(m_current_connection);
    OpenFilterPanelAdaptor<TagListBox>::clear(*m_tag_list_box);
    auto mode_blocker =
      boost::signals2::shared_connection_block(m_mode_connection);
    m_mode->set(Mode::INCLUDE);
    submit();
  }
}

#endif
