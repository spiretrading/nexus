#include "Spire/Ui/OpenFilterPanel.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/AssociativeValueModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Button.hpp"
#include "Spire/Ui/Checkbox.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/DropDownList.hpp"
#include "Spire/Ui/FilterPanel.hpp"
#include "Spire/Ui/Icon.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ScrollableListBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;
using namespace Styles;

namespace {
  auto DELETE_BUTTON_STYLE(StyleSheet style) {
    style.get((Hover() || Press()) / Body() >> is_a<Icon>()).
      set(BackgroundColor(QColor(Qt::transparent)));
    return style;
  }

  auto LABEL_STYLE(StyleSheet style) {
    style.get(Any()).set(PaddingRight(scale_width(8)));
    return style;
  }

  auto LIST_ITEM_STYLE(StyleSheet style) {
    style.get((Any() || Hover() || Press() || Focus() || Selected())).
      set(BackgroundColor(QColor(0xFFFFFF))).
      set(BorderSize(0, 0, 0, 0)).
      set(PaddingRight(scale_width(10)));
    return style;
  }
}

class DeletableItem : public QWidget {
  public:
    using DeleteSignal = Signal<void()>;

    explicit DeletableItem(QString label, QWidget* parent = nullptr) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->setSpacing(0);
      auto label_box = make_label(std::move(label));
      label_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      update_style(*label_box, [&] (auto& style) {
        style = LABEL_STYLE(style);
      });
      layout->addWidget(label_box);
      m_delete_button = make_delete_icon_button();
      m_delete_button->setFixedSize(scale(16, 16));
      m_delete_button->setFocusPolicy(Qt::NoFocus);
      update_style(*m_delete_button, [&] (auto& style) {
        style = DELETE_BUTTON_STYLE(style);
      });
      layout->addWidget(m_delete_button);
    }

    connection connect_delete_signal(
        const DeleteSignal::slot_type& slot) const {
      return m_delete_button->connect_clicked_signal(slot);
    }

  private:
    Button* m_delete_button;
};

class OpenFilterPanel::FilterModeButtonGroup {
  public:
    explicit FilterModeButtonGroup(
        std::shared_ptr<AssociativeValueModel<FilterMode>> current)
        : m_current(std::move(current)) {
      make_button(FilterMode::INCLUDE);
      make_button(FilterMode::EXCLUDE);
      on_update(m_current->get(), true);
    }

    const std::shared_ptr<AssociativeValueModel<FilterMode>>&
        get_current() const {
      return m_current;
    }

    CheckBox* get_button(FilterMode type) {
      return m_buttons[type];
    }

  private:
    std::shared_ptr<AssociativeValueModel<FilterMode>> m_current;
    std::unordered_map<FilterMode, CheckBox*> m_buttons;

    void on_update(FilterMode mode, bool value) {
      m_buttons[mode]->get_current()->set(value);
    }

    void make_button(FilterMode mode) {
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
      m_buttons[mode] = button;
    }

    QString display_text(FilterMode mode) {
      if(mode == FilterMode::INCLUDE) {
        static const auto value = tr("Include");
        return value;
      } else {
        static const auto value = tr("Exclude");
        return value;
      }
    }
};

struct OpenFilterQueryModel : OpenFilterPanel::QueryModel {
  std::shared_ptr<OpenFilterPanel::QueryModel> m_source;
  std::shared_ptr<ListModel> m_filtered;
  std::unordered_set<QString> m_filtered_set;
  scoped_connection m_filtered_connection;

  OpenFilterQueryModel(std::shared_ptr<OpenFilterPanel::QueryModel> source,
    std::shared_ptr<ListModel> filtered)
    : m_source(std::move(source)),
      m_filtered(std::move(filtered)),
      m_filtered_connection(m_filtered->connect_operation_signal(
        std::bind_front(&OpenFilterQueryModel::on_operation, this))) {}

  std::any parse(const QString& query) override {
    auto value = m_source->parse(query);
    if(!value.has_value()) {
      return value;
    }
    if(m_filtered_set.contains(displayTextAny(value))) {
      static auto result = std::any();
      return result;
    }
    return value;
  }

  QtPromise<std::vector<std::any>> submit(const QString& query) override {
    return m_source->submit(query).then([=] (auto&& source_result) {
      auto matches = [&] {
        try {
          return source_result.Get();
        } catch(const std::exception&) {
          return std::vector<std::any>();
        }
      }();
      auto result = std::vector<std::any>();
      for(auto& value : matches) {
        if(!m_filtered_set.contains(displayTextAny(value))) {
          result.push_back(value);
        }
      }
      return result;
    });
  }

  void on_operation(const ListModel::Operation& operation) {
    visit(operation,
      [&] (const ListModel::AddOperation& operation) {
        m_filtered_set.insert(
          displayTextAny(m_filtered->at(operation.m_index)));
      },
      [&] (const ListModel::RemoveOperation& operation) {
        m_filtered_set.clear();
        for(auto i = 0; i < m_filtered->get_size(); ++i) {
          m_filtered_set.insert(displayTextAny(m_filtered->at(i)));
        }
      });
  }
};

QWidget* OpenFilterPanel::default_combo_box_builder(
    const std::shared_ptr<QueryModel>& query_model,
    const SubmitHandler& submit_handler) {
  auto box = new ComboBox(query_model);
  box->connect_submit_signal(submit_handler);
  box->findChild<TextBox*>()->set_placeholder(tr("Type here"));
  return box;
}

OpenFilterPanel::OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
  QString title, QWidget& parent)
  : OpenFilterPanel(std::move(query_model), default_combo_box_builder,
      std::move(title), parent) {}

OpenFilterPanel::OpenFilterPanel(std::shared_ptr<QueryModel> query_model,
    ComboBoxBuilder combo_box_builder, QString title, QWidget& parent)
    : m_filtered(std::make_shared<ArrayListModel>()),
      m_mode_button_group(std::make_unique<FilterModeButtonGroup>(
        std::make_shared<AssociativeValueModel<FilterMode>>())) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto mode_layout = new QHBoxLayout();
  mode_layout->setContentsMargins({});
  mode_layout->setSpacing(scale_width(18));
  auto include_button = m_mode_button_group->get_button(FilterMode::INCLUDE);
  include_button->setFixedHeight(scale_height(16));
  mode_layout->addWidget(include_button);
  auto exclude_button = m_mode_button_group->get_button(FilterMode::EXCLUDE);
  exclude_button->setFixedHeight(scale_height(16));
  mode_layout->addWidget(exclude_button);
  m_mode_button_group->get_current()->connect_update_signal(
    std::bind_front(&OpenFilterPanel::on_mode_current, this));
  mode_layout->addStretch();
  layout->addLayout(mode_layout);
  layout->addSpacing(scale_height(18));
  m_combo_box = combo_box_builder(
    std::make_shared<OpenFilterQueryModel>(std::move(query_model), m_filtered),
    [=] (const auto& submission) {
      m_filtered->push(submission);
    });
  layout->addWidget(m_combo_box);
  layout->addSpacing(scale_height(8));
  auto list_view = new ListView(m_filtered,
    std::bind_front(&OpenFilterPanel::make_item, this));
  list_view->get_current()->connect_update_signal(
    std::bind_front(&OpenFilterPanel::on_list_view_current, this));
  m_filtered_connection = list_view->get_list()->connect_operation_signal(
    std::bind_front(&OpenFilterPanel::on_operation, this));
  m_scrollable_list_box = new ScrollableListBox(*list_view);
  m_scrollable_list_box->setMinimumSize(scale_width(164), scale_height(80));
  m_scrollable_list_box->setMaximumHeight(scale_height(158));
  layout->addWidget(m_scrollable_list_box);
  m_filter_panel = new FilterPanel(std::move(title), this, parent);
  m_filter_panel->connect_reset_signal(
    std::bind_front(&OpenFilterPanel::on_reset, this));
  window()->setWindowFlags(Qt::Tool | (window()->windowFlags() & ~Qt::Popup));
  window()->installEventFilter(this);
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
    m_combo_box->setFocus();
  } else if(event->type() == QEvent::HideToParent) {
    m_filter_panel->hide();
  }
  return QWidget::event(event);
}

QWidget* OpenFilterPanel::make_item(const std::shared_ptr<ListModel>& model,
    int index) {
  auto label = displayTextAny(model->at(index));
  auto item = new DeletableItem(label);
  item->connect_delete_signal([=] {
    auto remove_index = [&] {
      for(auto i = 0; i < m_filtered->get_size(); ++i) {
        if(label == displayTextAny(model->at(i))) {
          return i;
        }
      }
      return -1;
    }();
    if(remove_index >= 0) {
      m_filtered->remove(remove_index);
      m_combo_box->setFocus();
    }
  });
  return item;
}

void OpenFilterPanel::on_list_view_current(const optional<int>& current) {
  if(current) {
    m_scrollable_list_box->get_list_view().get_list_item(*current)->
      setFocusPolicy(Qt::NoFocus);
    m_scrollable_list_box->get_list_view().setFocusPolicy(Qt::NoFocus);
  }
}

void OpenFilterPanel::on_mode_current(FilterMode mode) {
  m_submit_signal(m_filtered, mode);
}

void OpenFilterPanel::on_operation(const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      auto item =
        m_scrollable_list_box->get_list_view().get_list_item(operation.m_index);
      update_style(*item, [&] (auto& style) {
        style = LIST_ITEM_STYLE(style);
      });
      item->setFocusPolicy(Qt::NoFocus);
      m_scrollable_list_box->get_list_view().setFocusPolicy(Qt::NoFocus);
      m_submit_signal(m_filtered, m_mode_button_group->get_current()->get());
    },
    [&] (const ListModel::RemoveOperation& operation) {
      m_submit_signal(m_filtered, m_mode_button_group->get_current()->get());
    });
}

void OpenFilterPanel::on_reset() {
  auto blocker = shared_connection_block(m_filtered_connection);
  while(m_filtered->get_size() != 0) {
    m_filtered->remove(m_filtered->get_size() - 1);
  }
  m_mode_button_group->get_current()->set(FilterMode::INCLUDE);
  m_combo_box->setFocus();
  m_submit_signal(m_filtered, m_mode_button_group->get_current()->get());
}
