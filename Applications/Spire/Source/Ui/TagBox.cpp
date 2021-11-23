#include "Spire/Ui/TagBox.hpp"
#include <QHBoxLayout>
#include <QKeyEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/ListItem.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/Tag.hpp"

using namespace boost::signals2;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto INPUT_BOX_STYLE(StyleSheet style) {
    style.get(Any()).
      set(vertical_padding(scale_height(3)));
    style.get(Any() >> is_a<ListView>()).
      set(ListItemGap(scale_width(4))).
      set(ListOverflowGap(scale_width(3))).
      set(Overflow::WRAP).
      set(Qt::Horizontal);
    style.get((ReadOnly() || Disabled()) >> is_a<ListView>()).
      set(Overflow::NONE);
    return style;
  }

  auto LIST_ITEM_STYLE() {
    auto style = StyleSheet();
    style.get(Any()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(padding(0));
    return style;
  }

  auto TEXT_BOX_STYLE(StyleSheet style) {
    style.get(Any() || Hover() || Focus() || Disabled()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(border_size(0)).
      set(horizontal_padding(0)).
      set(vertical_padding(scale_height(2)));
    return style;
  }
}

class PartialListModel : public ListModel {
  public:
    explicit PartialListModel(std::shared_ptr<ListModel> source)
      : m_source(std::move(source)) {}

    virtual int get_size() const {
      return m_source->get_size() + 1;
    }

    virtual const std::any& at(int index) const {
      if(index < 0 || index >= get_size()) {
        throw std::out_of_range("The index is out of range.");
      }
      if(index < m_source->get_size()) {
        return m_source->at(index);
      }
      static auto value = std::any();
      return value;
    }

    virtual QValidator::State set(int index, const std::any& value) {
      if(index < 0 || index >= get_size()) {
        return QValidator::State::Invalid;
      }
      if(index < m_source->get_size()) {
        return m_source->set(index, value);
      }
      return QValidator::State::Acceptable;
    }

    connection connect_operation_signal(
      const OperationSignal::slot_type& slot) const override {
      return m_source->connect_operation_signal(slot);
    }

  private:
    std::shared_ptr<ListModel> m_source;
};

TagBox::TagBox(std::shared_ptr<ListModel> list_model,
    std::shared_ptr<TextModel> current_model, QWidget* parent)
    : QWidget(parent),
      m_model(std::move(list_model)) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  m_text_box = new TextBox(std::move(current_model));
  set_style(*m_text_box, TEXT_BOX_STYLE(get_style(*m_text_box)));
  m_list_view = new ListView(std::make_shared<PartialListModel>(m_model),
    std::bind_front(&TagBox::build_tag, this));
  for(auto i = 0; i < m_list_view->get_list_model()->get_size(); ++i) {
    set_style(*m_list_view->get_list_item(i), LIST_ITEM_STYLE());
  }
  m_list_view->get_list_model()->connect_operation_signal(
    std::bind_front(&TagBox::on_operation, this));
  m_list_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  auto input_box = make_input_box(m_list_view);
  layout->addWidget(input_box);
  proxy_style(*this, *input_box);
  set_style(*this, INPUT_BOX_STYLE(get_style(*input_box)));
  setFocusProxy(m_list_view);
  setFocusPolicy(Qt::StrongFocus);
  m_text_box->findChild<QLineEdit*>()->installEventFilter(this);
}

const std::shared_ptr<ListModel>& TagBox::get_list_model() const {
  return m_model;
}

const std::shared_ptr<TextModel>& TagBox::get_current_model() const {
  return m_text_box->get_model();
}

bool TagBox::is_read_only() const {
  return m_text_box->is_read_only();
}

void TagBox::set_read_only(bool is_read_only) {
  m_text_box->set_read_only(is_read_only);
  update_tag_read_only();
  if(is_read_only) {
    match(*this, ReadOnly());
  } else {
    unmatch(*this, ReadOnly());
  }
}

connection TagBox::connect_delete_signal(
    const DeleteSignal::slot_type& slot) const {
  return m_delete_signal.connect(slot);
}

bool TagBox::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto& key_event = *static_cast<QKeyEvent*>(event);
    switch(key_event.key()) {
      case Qt::Key_Backspace:
        if(m_text_box->findChild<QLineEdit*>()->cursorPosition() == 0 &&
            m_model->get_size() > 0) {
          m_delete_signal(m_model->get_size() - 1);
        }
        break;
      case Qt::Key_Down:
      case Qt::Key_Up:
      case Qt::Key_PageDown:
      case Qt::Key_PageUp:
        return true;
      default:
        break;
    }
  }
  return QWidget::eventFilter(watched, event);
}

void TagBox::changeEvent(QEvent* event) {
  if(event->type() == QEvent::EnabledChange) {
    update_tag_read_only();
  }
  QWidget::changeEvent(event);
}

QWidget* TagBox::build_tag(const std::shared_ptr<ListModel>& model, int index) {
  if(index != model->get_size() - 1) {
    auto label = displayTextAny(model->at(index));
    auto tag = new Tag(label, this);
    tag->setFocusProxy(m_text_box);
    tag->set_read_only(m_text_box->is_read_only());
    tag->connect_delete_signal([=] {
      auto tag_index = [&] {
        for(auto i = 0; i < model->get_size(); ++i) {
          if(label == displayTextAny(model->at(i))) {
            return i;
          }
        }
        return -1;
      }();
      if(tag_index >= 0) {
        m_delete_signal(tag_index);
      }
    });
    connect(tag, &QWidget::destroyed, [=] { setFocus(); });
    m_tags.emplace_back(tag);
    return tag;
  }
  return m_text_box;
}

void TagBox::on_operation(const ListModel::Operation& operation) {
  visit(operation,
    [&] (const ListModel::AddOperation& operation) {
      auto item = m_list_view->get_list_item(operation.m_index);
      set_style(*item, LIST_ITEM_STYLE());
      setTabOrder(previousInFocusChain(), item);
      setTabOrder(item, m_text_box->nextInFocusChain());
      m_text_box->setFocusPolicy(Qt::StrongFocus);
    },
    [&] (const ListModel::RemoveOperation& operation) {
      m_tags.erase(m_tags.begin() + operation.m_index);
      if(m_tags.empty()) {
        m_list_view->setFocusProxy(m_text_box);
      }
    });
}

void TagBox::update_tag_read_only() {
  auto is_read_only = m_text_box->is_read_only() || !isEnabled();
  for(auto& tag : m_tags) {
    tag->set_read_only(is_read_only);
  }
}
