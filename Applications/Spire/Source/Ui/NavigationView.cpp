#include "Spire/Ui/NavigationView.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

class NavigationView::SelectLine : public QWidget {
  public:
    explicit SelectLine(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      auto box = new Box(nullptr);
      proxy_style(*this, *box);
      layout->addWidget(box);
    }
};

class NavigationView::Tab : public QWidget {
  public:
    explicit Tab(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      auto box = new Box(nullptr);
      proxy_style(*this, *box);
      layout->addWidget(box);
    }
};

class NavigationView::LabelContainer : public QWidget {
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

class NavigationView::NavigationTab : public QWidget {
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
      set_style(*this, std::move(style));
  }
};

class NavigationView::Separator : public QWidget {
  public:
    explicit Separator(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto layout = new QHBoxLayout(this);
      layout->setContentsMargins({});
      auto box = new Box(nullptr);
      proxy_style(*this, *box);
      layout->addWidget(box);
    }
};

NavigationView::NavigationView(QWidget* parent)
  : NavigationView(std::make_shared<LocalCurrentModel>(), parent) {}

NavigationView::NavigationView(std::shared_ptr<CurrentModel> current_model,
    QWidget* parent)
    : QWidget(parent),
      m_current_model(std::move(current_model)),
      m_current_connection(m_current_model->connect_current_signal(
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
  m_navigation_list = new ListView(std::make_shared<ArrayListModel>(),
    [&] (const auto& model, auto index) {
      return new NavigationTab(model->get<QString>(index));
    });
  m_navigation_list->setFixedHeight(scale_height(28));
  navigation_list_layout->addWidget(m_navigation_list);
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
  m_navigation_list->connect_submit_signal(
    std::bind_front(&NavigationView::on_submit, this));
}

const std::shared_ptr<NavigationView::CurrentModel>&
    NavigationView::get_current_model() const {
  return m_current_model;
}

void NavigationView::add_tab(QWidget* page, const QString& tab_label) {
  insert_tab(get_tab_count(), page, tab_label);
}

void NavigationView::insert_tab(int index, QWidget* page,
    const QString& tab_label) {
  if(index < 0 || index > get_tab_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  if(!page) {
    throw std::out_of_range("The page is null.");
  }
  static_pointer_cast<ArrayListModel>(m_navigation_list->get_list_model())->
    insert(tab_label, index);
  auto style = StyleSheet();
  style.get(Any()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border_color(QColor(Qt::transparent))).
    set(border_size(0)).
    set(padding(0));
  style.get((Hover() || Selected()) >> is_a<TextBox>()).
    set(TextColor(QColor(0x4B23A0)));
  style.get(Disabled() >> is_a<TextBox>()).
    set(TextColor(QColor(0xC8C8C8)));
  style.get(Checked() >> is_a<SelectLine>()).
    set(BackgroundColor(QColor(0x4B23A0)));
  style.get((Checked() && Disabled()) >> is_a<SelectLine>()).
    set(BackgroundColor(QColor(0xC8C8C8)));
  set_style(*m_navigation_list->get_list_item(index), std::move(style));
  m_stacked_widget->insertWidget(index, page);
  m_associative_model.get_association(tab_label)->connect_current_signal(
    std::bind_front(&NavigationView::on_associative_value_current, this, index));
}

int NavigationView::get_tab_count() const {
  return m_navigation_list->get_list_model()->get_size();
}

QString NavigationView::get_tab_label(int index) const {
  if(index < 0 || index >= get_tab_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_navigation_list->get_list_model()->get<QString>(index);
}

QWidget* NavigationView::get_page(int index) const {
  if(index < 0 || index >= get_tab_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_stacked_widget->widget(index);
}

bool NavigationView::is_tab_enabled(int index) const {
  if(index < 0 || index >= get_tab_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return m_navigation_list->get_list_item(index)->isEnabled();
}

void NavigationView::set_tab_enabled(int index, bool is_enabled) {
  if(index < 0 || index >= get_tab_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  if(!isEnabled()) {
    return;
  }
  m_navigation_list->get_list_item(index)->setEnabled(is_enabled);
  m_stacked_widget->widget(index)->setEnabled(is_enabled);
  if(is_enabled) {
    if(!m_navigation_list->isEnabled()) {
      m_navigation_list->setEnabled(true);
    }
    if(!m_stacked_widget->widget(m_current_model->get_current())->isEnabled()) {
      m_current_model->set_current(index);
    }
  } else if(!is_enabled && m_current_model->get_current() == index) {
    auto new_index = [=] {
      for(auto i = index + 1; i < get_tab_count(); ++i) {
        if(m_navigation_list->get_list_item(i)->isEnabled()) {
          return i;
        }
      }
      for(auto i = index - 1; i > -1; --i) {
        if(m_navigation_list->get_list_item(i)->isEnabled()) {
          return i;
        }
      }
      return -1;
    }();
    if(new_index > -1) {
      m_current_model->set_current(new_index);
    } else {
      m_navigation_list->setEnabled(false);
    }
  }
}

void NavigationView::on_current(int index) {
  if(index < 0 || index >= get_tab_count() || !isEnabled()) {
    return;
  }
  if(is_tab_enabled(index)) {
    m_associative_model.get_association(m_navigation_list->get_list_model()->
      get<QString>(index))->set_current(true);
  }
}

void NavigationView::on_submit(const std::any& submission) {
  m_associative_model.get_association(
    std::any_cast<QString>(submission))->set_current(true);
}

void NavigationView::on_associative_value_current(int index, bool value) {
  if(value) {
    match(*m_navigation_list->get_list_item(index), Checked());
    m_stacked_widget->setCurrentIndex(index);
    m_navigation_list->get_current_model()->set_current(index);
    if(index != m_current_model->get_current()) {
      m_current_model->set_current(index);
    }
  } else {
    unmatch(*m_navigation_list->get_list_item(index), Checked());
  }
}
