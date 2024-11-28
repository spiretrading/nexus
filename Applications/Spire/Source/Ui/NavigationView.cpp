#include "Spire/Ui/NavigationView.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CheckBox.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;
using namespace Styles;

namespace {
  using SelectLine = StateSelector<void, struct SelectLineTag>;
  using Separator = StateSelector<void, struct SeparatorTag>;
  using Tab = StateSelector<void, struct TabTag>;
}

class LabelContainer : public QWidget {
  public:
    explicit LabelContainer(QString label, QWidget* parent = nullptr)
        : QWidget(parent) {
      auto body = new QWidget();
      auto text_box = make_label(std::move(label));
      text_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      auto body_layout = make_vbox_layout(body);
      body_layout->addWidget(text_box);
      auto select_line = new Box();
      match(*select_line, SelectLine());
      select_line->setFixedHeight(scale_height(2));
      body_layout->addWidget(select_line);
      auto box = new Box(body);
      enclose(*this, *box);
      link(*this, *text_box);
      link(*this, *select_line);
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
      auto tab = new Box();
      match(*tab, Tab());
      tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      layers->add(tab);
      enclose(*this, *layers);
      link(*this, *container);
      link(*this, *tab);
      auto style = StyleSheet();
      style.get(Any() > is_a<LabelContainer>()).
        set(horizontal_padding(scale_width(8)));
      style.get(Any() > Tab()).
        set(border(scale_width(1), QColor(Qt::transparent)));
      style.get(FocusVisible() > Tab()).
        set(border_color(QColor(0x4B23A0)));
      style.get(FocusVisible() > is_a<LabelContainer>() > is_a<TextBox>()).
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
  auto navigation_menu = new QWidget();
  navigation_menu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_navigation_list = std::make_shared<ArrayListModel<std::any>>();
  m_navigation_view = new ListView(m_navigation_list,
    [] (const auto& model, auto index) {
      return new NavigationTab(
        std::any_cast<const QString&>(model->get(index)));
    });
  update_style(*m_navigation_view, [] (auto& style) {
    style.get(Any()).
      set(EdgeNavigation::CONTAIN).
      set(Overflow::NONE).
      set(Qt::Horizontal);
    style.get(Any() > is_a<ListItem>()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_color(QColor(Qt::transparent))).
      set(border_size(0)).
      set(padding(0));
    style.get(Any() > is_a<ListItem>() >
        is_a<NavigationTab>() > is_a<LabelContainer>() > SelectLine()).
      set(BackgroundColor(QColor(Qt::transparent)));
    style.get(Any() > (is_a<ListItem>() && (Checked() || Hover())) >
        is_a<NavigationTab>() > is_a<LabelContainer>() > is_a<TextBox>()).
      set(TextColor(QColor(0x4B23A0)));
    style.get(Any() > (is_a<ListItem>() && Disabled()) >
        is_a<NavigationTab>() > is_a<LabelContainer>() > is_a<TextBox>()).
      set(TextColor(QColor(0xC8C8C8)));
    style.get(Any() > (is_a<ListItem>() && Checked()) >
        is_a<NavigationTab>() > is_a<LabelContainer>() > SelectLine()).
      set(BackgroundColor(QColor(0x4B23A0)));
    style.get(Any() > (is_a<ListItem>() && Checked() && Disabled()) >
        is_a<NavigationTab>() > is_a<LabelContainer>() > SelectLine()).
      set(BackgroundColor(QColor(0xC8C8C8)));
  });
  m_navigation_view->setFixedHeight(scale_height(28));
  auto navigation_list_layout = make_hbox_layout();
  navigation_list_layout->addWidget(m_navigation_view);
  navigation_list_layout->addStretch();
  auto navigation_menu_layout = make_vbox_layout(navigation_menu);
  navigation_menu_layout->addSpacing(scale_height(7));
  navigation_menu_layout->addLayout(navigation_list_layout);
  auto separator = new Box();
  match(*separator, Separator());
  link(*this, *separator);
  separator->setFixedHeight(scale_height(5));
  navigation_menu_layout->addWidget(separator);
  auto layout = make_vbox_layout(this);
  layout->addWidget(navigation_menu);
  m_stacked_layout = new QStackedLayout();
  m_stacked_layout->setContentsMargins({});
  layout->addLayout(m_stacked_layout);
  auto style = StyleSheet();
  style.get(Any() > Separator()).
    set(BorderTopSize(scale_height(1))).
    set(BorderTopColor(QColor(0xD0D0D0)));
  set_style(*this, std::move(style));
  m_navigation_view->connect_submit_signal(
    std::bind_front(&NavigationView::on_list_submit, this));
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
  auto content_block = new QWidget();
  auto size_policy = page.sizePolicy();
  auto aligment = Qt::Alignment();
  if(size_policy.horizontalPolicy() == QSizePolicy::Preferred ||
      size_policy.horizontalPolicy() == QSizePolicy::Fixed) {
    aligment |= Qt::AlignLeft;
  }
  if(size_policy.verticalPolicy() == QSizePolicy::Preferred ||
      size_policy.verticalPolicy() == QSizePolicy::Fixed) {
    aligment |= Qt::AlignTop;
  }
  auto layout = make_vbox_layout(content_block);
  layout->addWidget(&page, 0, aligment);
  m_stacked_layout->insertWidget(index, content_block);
  m_associative_model.get_association(label)->connect_update_signal(
    std::bind_front(
      &NavigationView::on_associative_value_current, this, index));
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
  return std::any_cast<QString>(m_navigation_list->get(index));
}

QWidget& NavigationView::get_page(int index) const {
  if(index < 0 || index >= get_count()) {
    throw std::out_of_range("The index is out of range.");
  }
  return *m_stacked_layout->widget(index);
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
  m_stacked_layout->widget(index)->setEnabled(is_enabled);
  if(is_enabled) {
    if(!m_navigation_view->isEnabled()) {
      m_navigation_view->setEnabled(true);
    }
    if(!m_stacked_layout->currentWidget()->isEnabled()) {
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
    std::any_cast<const QString&>(m_navigation_list->get(index)))->set(true);
}

void NavigationView::on_list_submit(const std::any& submission) {
  m_associative_model.get_association(
    std::any_cast<const QString&>(submission))->set(true);
}

void NavigationView::on_associative_value_current(int index, bool value) {
  if(value) {
    match(*m_navigation_view->get_list_item(index), Checked());
    m_stacked_layout->setCurrentIndex(index);
    m_navigation_view->get_current()->set(index);
    if(index != m_current->get()) {
      m_current->set(index);
    }
  } else {
    unmatch(*m_navigation_view->get_list_item(index), Checked());
  }
}
