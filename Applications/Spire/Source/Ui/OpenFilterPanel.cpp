#include "Spire/Ui/OpenFilterPanel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/AnyInputBox.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace signals2;
using namespace Spire;
using namespace Styles;

namespace {
  auto DELETE_BUTTON_STYLE(StyleSheet style) {
    style.get(Any() > is_a<TextBox>()).set(PaddingRight(scale_width(8)));
    style.get((Hover() || Press()) > Body() > is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Disabled() > Body() > is_a<Icon>()).set(Visibility::NONE);
    return style;
  }

  auto LIST_VIEW_STYLE(StyleSheet style) {
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(border_size(0)).
      set(PaddingRight(scale_width(10)));
    return style;
  }

  auto& display_text(OpenFilterPanel::Mode mode) {
    if(mode == OpenFilterPanel::Mode::INCLUDE) {
      static const auto value = QObject::tr("Include");
      return value;
    } else {
      static const auto value = QObject::tr("Exclude");
      return value;
    }
  }
}

class DeletableItem : public QWidget {
  public:
    using DeleteSignal = Signal<void ()>;

    explicit DeletableItem(QString label, QWidget* parent = nullptr)
        : QWidget(parent) {
      auto label_box = make_label(std::move(label));
      label_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      auto layout = make_hbox_layout(this);
      layout->addWidget(label_box);
      m_delete_button = make_delete_icon_button();
      m_delete_button->setFixedSize(scale(16, 16));
      m_delete_button->setFocusPolicy(Qt::NoFocus);
      update_style(*m_delete_button, [] (auto& style) {
        style = DELETE_BUTTON_STYLE(style);
      });
      layout->addWidget(m_delete_button);
    }

    connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const {
      return m_delete_button->connect_click_signal(slot);
    }

  private:
    Button* m_delete_button;
};

struct ComboBoxFilterQueryModel : ComboBox::QueryModel {
  std::shared_ptr<ComboBox::QueryModel> m_source;
  std::shared_ptr<AnyListModel> m_matches;
  std::unordered_set<QString> m_matches_set;
  std::vector<QString> m_matches_list;
  scoped_connection m_matches_connection;

  ComboBoxFilterQueryModel(std::shared_ptr<ComboBox::QueryModel> source,
      std::shared_ptr<AnyListModel> matches)
      : m_source(std::move(source)),
        m_matches(std::move(matches)),
        m_matches_connection(m_matches->connect_operation_signal(
          std::bind_front(&ComboBoxFilterQueryModel::on_operation, this))) {
    for(auto i = 0; i < m_matches->get_size(); ++i) {
      add_match(i);
    }
  }

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_matches_set.contains(displayText(value))) {
      return std::any();
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
      auto result = [&] {
        try {
          return source_result.Get();
        } catch(const std::exception&) {
          return std::vector<std::any>();
        }
      }();
      std::erase_if(result, [&] (const auto& value) {
        return m_matches_set.contains(displayText(value));
      });
      return result;
    });
  }

  void on_operation(const AnyListModel::Operation& operation) {
    visit(operation,
      [&] (const AnyListModel::AddOperation& operation) {
        add_match(operation.m_index);
      },
      [&] (const AnyListModel::RemoveOperation& operation) {
        m_matches_set.erase(m_matches_list[operation.m_index]);
        m_matches_list.erase(m_matches_list.begin() + operation.m_index);
      });
  }

  void add_match(int index) {
    auto value = displayText(m_matches->get(index));
    m_matches_set.insert(value);
    m_matches_list.insert(m_matches_list.begin() + index, value);
  }
};

class OpenFilterPanel::FilterModeButtonGroup {
  public:
    explicit FilterModeButtonGroup(std::shared_ptr<ValueModel<Mode>> model)
        : m_model(std::move(model)),
          m_current(
            std::make_shared<AssociativeValueModel<Mode>>(m_model->get())),
          m_connection(m_model->connect_update_signal(
            std::bind_front(&FilterModeButtonGroup::on_model_update, this))) {
      make_button(Mode::INCLUDE);
      make_button(Mode::EXCLUDE);
      on_update(m_current->get(), true);
    }

    const std::shared_ptr<ValueModel<Mode>>& get_model() const {
      return m_model;
    }

    CheckBox* get_button(Mode mode) {
      return m_buttons[static_cast<int>(mode)];
    }

  private:
    std::shared_ptr<ValueModel<Mode>> m_model;
    std::shared_ptr<AssociativeValueModel<Mode>> m_current;
    std::array<CheckBox*, 2> m_buttons;
    scoped_connection m_connection;

    void on_update(Mode mode, bool value) {
      get_button(mode)->get_current()->set(value);
      if(value) {
        auto blocker = shared_connection_block(m_connection);
        m_model->set(mode);
      }
    }

    void on_model_update(Mode mode) {
      m_current->set(mode);
    }

    void make_button(Mode mode) {
      auto button = make_radio_button();
      button->set_label(display_text(mode));
      button->get_current()->connect_update_signal([=] (auto value) {
        if(m_current->get() == mode && !value) {
          button->get_current()->set(true);
        } else if(value) {
          m_current->set(mode);
        }
      });
      m_current->get_association(mode)->connect_update_signal(
        std::bind_front(&FilterModeButtonGroup::on_update, this, mode));
      m_buttons[static_cast<int>(mode)] = button;
    }
};

AnyInputBox* combo_box_builder(std::shared_ptr<ComboBox::QueryModel> model,
    std::shared_ptr<AnyListModel> matches) {
  auto box = new ComboBox(
    std::make_shared<ComboBoxFilterQueryModel>(model, matches));
  box->set_placeholder(QObject::tr("Type here"));
  auto input_box = new AnyInputBox(*box);
  input_box->connect_submit_signal([=] (const auto& submission) {
    input_box->get_current()->set(std::any());
    matches->push(any_cast<std::any>(submission));
  });
  return input_box;
}

OpenFilterPanel::OpenFilterPanel(
  std::shared_ptr<ComboBox::QueryModel> query_model, QString title,
  QWidget& parent)
  : OpenFilterPanel(std::bind_front(&combo_box_builder, std::move(query_model)),
      std::move(title), parent) {}

OpenFilterPanel::OpenFilterPanel(
  InputBoxBuilder input_box_builder, QString title, QWidget& parent)
  : OpenFilterPanel(std::move(input_box_builder),
    std::make_shared<ArrayListModel<std::any>>(),
    std::make_shared<LocalValueModel<Mode>>(Mode::INCLUDE),
    std::move(title), parent) {}

OpenFilterPanel::OpenFilterPanel(InputBoxBuilder input_box_builder,
    std::shared_ptr<AnyListModel> matches,
    std::shared_ptr<ValueModel<Mode>> mode, QString title, QWidget& parent)
    : QWidget(&parent),
      m_matches(std::move(matches)),
      m_mode(std::move(mode)),
      m_mode_button_group(std::make_unique<FilterModeButtonGroup>(m_mode)) {
  auto include_button = m_mode_button_group->get_button(Mode::INCLUDE);
  include_button->setFixedHeight(scale_height(16));
  auto mode_layout = make_hbox_layout();
  mode_layout->setSpacing(scale_width(18));
  mode_layout->addWidget(include_button);
  auto exclude_button = m_mode_button_group->get_button(Mode::EXCLUDE);
  exclude_button->setFixedHeight(scale_height(16));
  mode_layout->addWidget(exclude_button);
  m_mode_connection = m_mode_button_group->get_model()->connect_update_signal(
    std::bind_front(&OpenFilterPanel::on_mode_current, this));
  mode_layout->addStretch();
  auto layout = make_vbox_layout(this);
  layout->addLayout(mode_layout);
  layout->addSpacing(scale_height(18));
  m_input_box = input_box_builder(m_matches);
  layout->addWidget(m_input_box);
  layout->addSpacing(scale_height(8));
  m_list_view = new ListView(m_matches,
    std::bind_front(&OpenFilterPanel::make_item, this));
  update_style(*m_list_view, [] (auto& style) {
    style = LIST_VIEW_STYLE(style);
  });
  m_matches_connection = m_matches->connect_operation_signal(
    std::bind_front(&OpenFilterPanel::on_matches_operation, this));
  auto scrollable_list_box = new ScrollableListBox(*m_list_view);
  scrollable_list_box->setMinimumSize(scale_width(164), scale_height(80));
  scrollable_list_box->setMaximumHeight(scale_height(158));
  layout->addWidget(scrollable_list_box);
  m_filter_panel = new FilterPanel(std::move(title), this, parent);
  m_filter_panel->connect_reset_signal(
    std::bind_front(&OpenFilterPanel::on_reset, this));
  auto panel_window = window();
  panel_window->setWindowFlags(Qt::Tool | panel_window->windowFlags());
  panel_window->installEventFilter(this);
}

const std::shared_ptr<AnyListModel>& OpenFilterPanel::get_matches() const {
  return m_matches;
}

const std::shared_ptr<ValueModel<OpenFilterPanel::Mode>>&
    OpenFilterPanel::get_mode() const {
  return m_mode;
}

connection OpenFilterPanel::connect_submit_signal(
    const SubmitSignal::slot_type& slot) const {
  return m_submit_signal.connect(slot);
}

bool OpenFilterPanel::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::Close) {
    m_filter_panel->hide();
    hide();
  }
  return QWidget::eventFilter(watched, event);
}

bool OpenFilterPanel::event(QEvent* event) {
  if(event->type() == QEvent::ShowToParent) {
    m_filter_panel->show();
    window()->activateWindow();
  } else if(event->type() == QEvent::HideToParent) {
    m_filter_panel->hide();
  } else if(event->type() == QEvent::FocusIn) {
    m_input_box->setFocus();
  }
  return QWidget::event(event);
}

QWidget* OpenFilterPanel::make_item(const std::shared_ptr<AnyListModel>& model,
    int index) {
  auto label = displayText(model->get(index));
  auto item = new DeletableItem(label);
  item->connect_delete_signal([=] {
    auto index = [&] {
      for(auto i = 0; i < m_matches->get_size(); ++i) {
        if(label == displayText(model->get(i))) {
          return i;
        }
      }
      return -1;
    }();
    if(index >= 0) {
      m_matches->remove(index);
      m_input_box->setFocus();
    }
  });
  return item;
}

void OpenFilterPanel::on_mode_current(Mode mode) {
  submit();
}

void OpenFilterPanel::on_matches_operation(
    const AnyListModel::Operation& operation) {
  visit(operation,
    [&] (const AnyListModel::AddOperation& operation) {
      submit();
    },
    [&] (const AnyListModel::RemoveOperation& operation) {
      submit();
    });
}

void OpenFilterPanel::on_reset() {
  auto matches_blocker = shared_connection_block(m_matches_connection);
  while(m_matches->get_size() != 0) {
    m_matches->remove(m_matches->get_size() - 1);
  }
  auto mode_blocker = shared_connection_block(m_mode_connection);
  m_mode_button_group->get_model()->set(Mode::INCLUDE);
  m_input_box->setFocus();
  submit();
}

void OpenFilterPanel::submit() {
  if(m_matches->get_size() == 0) {
    m_submit_signal(m_matches, Mode::EXCLUDE);
  } else {
    m_submit_signal(m_matches, m_mode->get());
  }
}
