#include "Spire/Ui/NavigationView.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

class SelectLine : public Box {
  public:
    explicit SelectLine(QWidget* parent = nullptr)
      : Box(nullptr, parent) {}
};

class Separator : public Box {
  public:
    explicit Separator(QWidget* parent = nullptr)
      : Box(nullptr, parent) {}
};

class Tab : public Box {
  public:
    explicit Tab(QWidget* parent = nullptr)
      : Box(nullptr, parent) {}
};

class LabelContainer : public QWidget {
  public:
    explicit LabelContainer(QString label, QWidget* parent = nullptr)
        : QWidget(parent) {
      auto body = new QWidget();
      auto body_layout = new QVBoxLayout(body);
      body_layout->setContentsMargins({});
      body_layout->setSpacing(0);
      auto text_box = make_label(std::move(label));
      text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      body_layout->addWidget(text_box);
      auto select_line = new SelectLine();
      select_line->setFixedHeight(scale_height(2));
      body_layout->addWidget(select_line);
      auto box = new Box(body);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->addWidget(box);
      proxy_style(*this, *box);
    }
};

class NavigationTab : public QWidget {
  public:
    explicit NavigationTab(QString label, QWidget* parent = nullptr)
        : QWidget(parent) {
      setFocusPolicy(Qt::ClickFocus);
      auto layers = new LayeredWidget();
      layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      auto container = new LabelContainer(std::move(label));
      container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      layers->add(container);
      auto tab = new Tab();
      tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      layers->add(tab);
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      layout->addWidget(layers);
      auto style = StyleSheet();
      style.get(Any() >> is_a<LabelContainer>()).
        set(horizontal_padding(scale_width(8)));
      style.get(Any() >> is_a<Tab>()).
        set(border(scale_width(1), QColor(Qt::transparent)));
      style.get(FocusVisible() >> is_a<Tab>()).
        set(border_color(QColor(0x4B23A0)));
      style.get(FocusVisible() >> is_a<TextBox>()).
        set(TextColor(QColor(0x4B23A0)));
      set_style(*this, std::move(style));
    }
};

NavigationView::NavigationView(QWidget* parent)
  : NavigationView(std::make_shared<LocalValueModel<int>>(), parent) {}

NavigationView::NavigationView(
    std::shared_ptr<CurrentModel> current, QWidget* parent)
    : QWidget(parent),
      m_current(std::move(current)),
      m_current_connection(m_current->connect_update_signal(
        std::bind_front(&NavigationView::on_current, this))) {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  auto navigation_menu = new QWidget();
  navigation_menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  auto navigation_menu_layout = new QVBoxLayout(navigation_menu);
  navigation_menu_layout->setContentsMargins({});
  navigation_menu_layout->setSpacing(0);
  navigation_menu_layout->addSpacing(scale_height(7));
  auto navigation_list_layout = new QHBoxLayout();
  navigation_list_layout->setContentsMargins({});
  navigation_list_layout->setSpacing(0);
  m_navigation_list = std::make_shared<ArrayListModel>();
  m_navigation_view = new ListView(m_navigation_list,
    [] (const auto& model, auto index) {
      return new NavigationTab(model->get<QString>(index));
    });
  m_navigation_view->setFixedHeight(scale_height(28));
  navigation_list_layout->addWidget(m_navigation_view);
  navigation_list_layout->addStretch();
  navigation_menu_layout->addLayout(navigation_list_layout);
  auto separator = new Separator();
  separator->setFixedHeight(scale_height(5));
  navigation_menu_layout->addWidget(separator);
  layout->addWidget(navigation_menu);
  auto content_block_layout = new QHBoxLayout();
  content_block_layout->setContentsMargins({});
  content_block_layout->setSpacing(0);
  auto content_layout = new QVBoxLayout();
  content_layout->setContentsMargins({});
  content_layout->setSpacing(0);
  m_stacked_widget = new QStackedWidget();
  m_stacked_widget->setContentsMargins({});
  content_layout->addWidget(m_stacked_widget);
  content_layout->addStretch();
  content_block_layout->addLayout(content_layout);
  content_block_layout->addStretch();
  layout->addLayout(content_block_layout);
  auto style = StyleSheet();
  style.get(Any() >> is_a<ListView>()).
    set(EdgeNavigation::CONTAIN).
    set(Overflow::NONE).
    set(Qt::Horizontal).
    set(SelectionMode::SINGLE);
  style.get(Any() >> is_a<Separator>()).
    set(BorderTopSize(scale_height(1))).
    set(BorderTopColor(QColor(0xD0D0D0)));
  set_style(*this, std::move(style));
  m_navigation_view->connect_submit_signal(
    std::bind_front(&NavigationView::on_list_submit, this));
  m_navigation_view->get_current()->connect_update_signal(
    std::bind_front(&NavigationView::on_list_current, this));
}

const std::shared_ptr<NavigationView::CurrentModel>&
    NavigationView::get_current() const {
  return m_current;
}

void NavigationView::add_tab(QWidget& page, const QString& label) {
  insert_tab(get_count(), page, label);
}

void NavigationView::insert_tab(int index, QWidget& page,
    const QString& label) {
  if(index < 0 || index > get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  m_navigation_list->insert(label, index);
  auto style = StyleSheet();
  style.get(Any()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border_color(QColor(Qt::transparent))).
    set(border_size(0)).
    set(padding(0));
  style.get((Checked() || Hover()) >> is_a<TextBox>()).
    set(TextColor(QColor(0x4B23A0)));
  style.get(Disabled() >> is_a<TextBox>()).
    set(TextColor(QColor(0xC8C8C8)));
  style.get(Checked() >> is_a<SelectLine>()).
    set(BackgroundColor(QColor(0x4B23A0)));
  style.get((Checked() && Disabled()) >> is_a<SelectLine>()).
    set(BackgroundColor(QColor(0xC8C8C8)));
  set_style(*m_navigation_view->get_list_item(index), std::move(style));
  m_stacked_widget->insertWidget(index, &page);
  m_associative_model.get_association(label)->connect_update_signal(
    std::bind_front(&NavigationView::on_associative_value_current, this, index));
  if(index == m_current->get()) {
    on_current(index);
  }
}

int NavigationView::get_count() const {
  return m_navigation_list->get_size();
}

QString NavigationView::get_label(int index) const {
  if(index < 0 || index >= get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_navigation_list->get<QString>(index);
}

QWidget& NavigationView::get_page(int index) const {
  if(index < 0 || index >= get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return *m_stacked_widget->widget(index);
}

bool NavigationView::is_enabled(int index) const {
  if(index < 0 || index >= get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_navigation_view->get_list_item(index)->isEnabled();
}

void NavigationView::set_enabled(int index, bool is_enabled) {
  if(index < 0 || index >= get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  if(!isEnabled()) {
    return;
  }
  m_navigation_view->get_list_item(index)->setEnabled(is_enabled);
  m_stacked_widget->widget(index)->setEnabled(is_enabled);
  if(is_enabled) {
    if(!m_navigation_view->isEnabled()) {
      m_navigation_view->setEnabled(true);
    }
    if(!m_stacked_widget->currentWidget()->isEnabled()) {
      m_current->set(index);
    }
  } else if(m_current->get() == index) {
    auto new_index = [=] {
      for(auto i = index + 1; i < get_count(); ++i) {
        if(m_navigation_view->get_list_item(i)->isEnabled()) {
          return i;
        }
      }
      for(auto i = index - 1; i > -1; --i) {
        if(m_navigation_view->get_list_item(i)->isEnabled()) {
          return i;
        }
      }
      return -1;
    }();
    if(new_index > -1) {
      m_current->set(new_index);
    } else {
      m_navigation_view->setEnabled(false);
    }
  }
}

void NavigationView::on_current(int index) {
  if(index < 0 || index >= get_count()) {
    return;
  }
  m_associative_model.get_association(
    m_navigation_list->get<QString>(index))->set(true);
}

void NavigationView::on_list_submit(const std::any& submission) {
  m_associative_model.get_association(
    std::any_cast<QString>(submission))->set(true);
}

void NavigationView::on_list_current(const optional<int>& current) {
  if(current) {
    m_stacked_widget->setFocusPolicy(Qt::TabFocus);
    setTabOrder(m_stacked_widget, m_navigation_view->get_list_item(*current));
    m_stacked_widget->setFocusPolicy(Qt::NoFocus);
  }
}

void NavigationView::on_associative_value_current(int index, bool value) {
  if(value) {
    match(*m_navigation_view->get_list_item(index), Checked());
    m_stacked_widget->setCurrentIndex(index);
    m_navigation_view->get_current()->set(index);
    if(index != m_current->get()) {
      m_current->set(index);
    }
  } else {
    unmatch(*m_navigation_view->get_list_item(index), Checked());
  }
}
