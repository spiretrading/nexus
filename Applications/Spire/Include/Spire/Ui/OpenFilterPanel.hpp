#ifndef SPIRE_OPEN_FILTER_PANEL_HPP
#define SPIRE_OPEN_FILTER_PANEL_HPP
#include <boost/signals2/shared_connection_block.hpp>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/RegionBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"

namespace Spire {
namespace Details {
  template<typename T>
  struct TagComboBoxTraits;

  template<typename T>
  struct TagComboBoxTraits<TagComboBox<T>> {
    using SubmissionType = std::shared_ptr<ListModel<T>>;

    static bool is_empty(TagComboBox<T>& combo_box) {
      return combo_box.get_current()->get_size() == 0;
    }

    static void clear(TagComboBox<T>& combo_box) {
      Spire::clear(*combo_box.get_current());
    }

    static SubmissionType get_current(TagComboBox<T>& combo_box) {
      return combo_box.get_current();
    }

    static boost::signals2::connection connect_current(
        TagComboBox<T>& combo_box, const std::function<void()>& slot) {
      return combo_box.get_current()->connect_operation_signal(
        [=] (const auto& operation) {
          visit(operation,
            [&] (const ListModel<QString>::AddOperation& operation) {
              slot();
            },
            [&] (const ListModel<QString>::RemoveOperation& operation) {
              slot();
            });
        });
    }
  };

  template<>
  struct TagComboBoxTraits<RegionBox> {
    using SubmissionType = Nexus::Region;

    static bool is_empty(RegionBox& combo_box) {
      return combo_box.get_current()->get().is_empty();
    }

    static void clear(RegionBox& combo_box) {
      combo_box.get_current()->set(Nexus::Region());
    }

    static SubmissionType get_current(RegionBox& combo_box) {
      return combo_box.get_current()->get();
    }

    static boost::signals2::connection connect_current(
        RegionBox& combo_box, const std::function<void()>& slot) {
      return combo_box.get_current()->connect_update_signal([=] (const auto&) {
        slot();
      });
    }
  };
}

  /** Displays a FilterPanel over an open list of values. */
  template<typename T>
  class OpenFilterPanel : public QWidget {
    public:

      /** The type of the TagComboBox-like component. */
      using TagComboBox = T;

      /** The type of the submission produced by the TagComboBox. */
      using SubmissionType =
        typename Details::TagComboBoxTraits<T>::SubmissionType;

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
       * @param tag_combo_box The TagComboBox-like component used as the input.
       * @param parent The parent widget.
       */
      explicit OpenFilterPanel(TagComboBox& tag_combo_box,
        QWidget* parent = nullptr);

      /** Returns the TagComboBox-like component. */
      const TagComboBox& get_tag_combo_box() const;

      /** Returns the TagComboBox-like component. */
      TagComboBox& get_tag_combo_box();

      /** Connects a slot to the submit signal. */
      boost::signals2::connection connect_submit_signal(
        const SubmitSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;
      void showEvent(QShowEvent* event) override;

    private:
      mutable SubmitSignal m_submit_signal;
      TagComboBox* m_tag_combo_box;
      std::shared_ptr<AssociativeValueModel<Mode>> m_mode;
      QWidget* m_body;
      QWidget* m_button_container;
      boost::signals2::scoped_connection m_current_connection;
      boost::signals2::scoped_connection m_mode_connection;

      QString display_text(Mode mode) const;
      CheckBox* make_mode_check_box(Mode mode) const;
      QLayout* make_mode_section_layout(Mode mode) const;
      void submit();
      void on_current();
      void on_mode(Mode mode);
      void on_reset();
  };
  
  template<typename T>
  OpenFilterPanel<T>::OpenFilterPanel(TagComboBox& tag_combo_box,
      QWidget* parent)
      : QWidget(parent),
        m_tag_combo_box(&tag_combo_box),
        m_mode(std::make_shared<AssociativeValueModel<Mode>>(Mode::INCLUDE)) {
    m_tag_combo_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_tag_combo_box->setMinimumSize(scale(160, 26));
    m_tag_combo_box->setMaximumHeight(scale_height(63));
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
    body_layout->addWidget(m_button_container);
    body_layout->setSpacing(scale_width(18));
    body_layout->addWidget(m_tag_combo_box);
    auto panel = new FilterPanel(*m_body);
    enclose(*this, *panel);
    Styles::proxy_style(*this, *panel);
    setFocusProxy(m_tag_combo_box);
    panel->connect_reset_signal(
      std::bind_front(&OpenFilterPanel<TagComboBox>::on_reset, this));
    m_current_connection =
      Details::TagComboBoxTraits<TagComboBox>::connect_current(
        *m_tag_combo_box,
        std::bind_front(&OpenFilterPanel<TagComboBox>::on_current, this));
    m_mode_connection = m_mode->connect_update_signal(
      std::bind_front(&OpenFilterPanel<TagComboBox>::on_mode, this));
  }

  template<typename T>
  const OpenFilterPanel<T>::TagComboBox&
      OpenFilterPanel<T>::get_tag_combo_box() const {
    return *m_tag_combo_box;
  }

  template<typename T>
  OpenFilterPanel<T>::TagComboBox& OpenFilterPanel<T>::get_tag_combo_box() {
    return *m_tag_combo_box;
  }

  template<typename T>
  boost::signals2::connection OpenFilterPanel<T>::connect_submit_signal(
      const SubmitSignal::slot_type& slot) const {
    return m_submit_signal.connect(slot);
  }

  template<typename T>
  bool OpenFilterPanel<T>::eventFilter(QObject* watched, QEvent* event) {
    if(watched == m_body && event->type() == QEvent::Resize) {
      m_tag_combo_box->setMaximumWidth(
        m_body->width() - m_button_container->width() - scale_width(18));
    }
    return QWidget::eventFilter(watched, event);
  }

  template<typename T>
  void OpenFilterPanel<T>::showEvent(QShowEvent* event) {
    auto next = m_tag_combo_box->nextInFocusChain();
    while(next != m_tag_combo_box) {
      next = next->nextInFocusChain();
      if(!m_body->isAncestorOf(next) && next->focusPolicy() & Qt::TabFocus) {
        break;
      }
    }
    setTabOrder(find_focus_proxy(*m_tag_combo_box), next);
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
  void OpenFilterPanel<T>::submit() {
    auto mode = [&] {
      if(Details::TagComboBoxTraits<TagComboBox>::is_empty(*m_tag_combo_box)) {
        return Mode::EXCLUDE;
      }
      return m_mode->get();
    }();
    m_submit_signal(
      Details::TagComboBoxTraits<TagComboBox>::get_current(*m_tag_combo_box),
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
    Details::TagComboBoxTraits<TagComboBox>::clear(*m_tag_combo_box);
    auto mode_blocker =
      boost::signals2::shared_connection_block(m_mode_connection);
    m_mode->set(Mode::INCLUDE);
    submit();
  }

  using RegionFilterPanel = OpenFilterPanel<RegionBox>;
}

#endif
