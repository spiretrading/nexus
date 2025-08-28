#include "Spire/KeyBindings/OrderFieldInfoTip.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/InfoTip.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextAreaBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  auto NAME_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setWeight(QFont::Medium);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly()).
      set(Font(font)).
      set(TextColor(QColor(0x808080)));
    return style;
  }

  auto PREREQUISITES_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setPixelSize(scale_width(12));
    font.setItalic(true);
    style.get(ReadOnly()).
      set(border_size(0)).
      set(Font(font)).
      set(TextColor(QColor(0x4B23A0))).
      set(PaddingLeft(scale_width(18))).
      set(PaddingTop(scale_height(8))).
      set(vertical_padding(0));
    return style;
  }

  auto make_description_layout(const OrderFieldInfoTip::Model& model) {
    auto name_label = make_label(QString::fromStdString(model.m_tag.m_name));
    name_label->set_read_only(true);
    set_style(*name_label, NAME_STYLE());
    auto description_label =
      new TextAreaBox(QString::fromStdString(model.m_tag.m_description));
    description_label->set_read_only(true);
    update_style(*description_label, [] (auto& style) {
      style.get(ReadOnly()).
        set(PaddingTop(scale_height(18))).
        set(PaddingBottom(0));
    });
    auto layout = make_vbox_layout();
    layout->setContentsMargins(
      scale_width(18), scale_height(18), scale_width(18), scale_height(18));
    layout->addWidget(name_label);
    layout->addWidget(description_label);
    return layout;
  }

  auto make_value_row(const OrderFieldInfoTip::Model::Argument& argument) {
    auto id_label = make_label(QString::fromStdString(argument.m_value));
    auto description_label =
      new TextAreaBox(QString::fromStdString(argument.m_description));
    description_label->set_read_only(true);
    description_label->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*description_label, [] (auto& style) {
      style.get(ReadOnly()).
        set(border_size(0)).
        set(vertical_padding(0)).
        set(PaddingLeft(scale_width(8)));
    });
    return std::tuple(id_label, description_label);
  }

  auto make_values_table(
      const std::vector<OrderFieldInfoTip::Model::Argument>& arguments) {
    auto layout = make_grid_layout();
    layout->setContentsMargins(
      scale_width(18), 0, scale_width(18), scale_height(18));
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(scale_height(12));
    for(auto i = std::size_t(0); i != arguments.size(); ++i) {
      auto& value = arguments[i];
      auto [id, description] = make_value_row(value);
      layout->addWidget(id, i, 0, Qt::AlignTop);
      layout->addWidget(description, i, 1, Qt::AlignTop);
    }
    return layout;
  }

  auto make_scroll_box_body(const OrderFieldInfoTip::Model& model) {
    auto body = new QWidget();
    body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_vbox_layout(body);
    layout->addLayout(make_description_layout(model));
    if(!model.m_tag.m_arguments.empty()) {
      layout->addLayout(make_values_table(model.m_tag.m_arguments));
    }
    return body;
  }

  QString describe_prerequisites(const OrderFieldInfoTip::Model& model) {
    auto text = QString();
    for(auto tag_index = std::size_t(0);
        tag_index < model.m_prerequisites.size(); ++tag_index) {
      auto tag = model.m_prerequisites[tag_index];
      text.append(QString::fromStdString(tag.m_name));
      if(!tag.m_arguments.empty()) {
        text.append(" = \"");
        for(auto i = std::size_t(0); i < tag.m_arguments.size(); ++i) {
          text.append(QString::fromStdString(tag.m_arguments[i].m_value));
          if(i < tag.m_arguments.size() - 1) {
            text.append(" | ");
          }
        }
        text.append("\"");
      }
      if(tag_index < model.m_prerequisites.size() - 1) {
        text.append(", ");
      }
    }
    return text;
  }

  auto make_prerequisite_container(const OrderFieldInfoTip::Model& model) {
    auto prerequisites_label = new TextAreaBox(describe_prerequisites(model));
    prerequisites_label->set_read_only(true);
    set_style(*prerequisites_label, PREREQUISITES_STYLE());
    auto container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_vbox_layout(container);
    layout->addWidget(make_label(QObject::tr("Prerequisites")));
    layout->addWidget(prerequisites_label);
    auto box = new Box(container);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*box, [&] (auto& style) {
      style.get(Any()).
        set(BorderTopColor(QColor(0xC8C8C8))).
        set(BorderTopSize(scale_height(1))).
        set(padding(scale_width(18)));
    });
    return box;
  }
}

OrderFieldInfoTip::OrderFieldInfoTip(Model model, QWidget* parent)
    : QWidget(parent) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto layout = make_vbox_layout(this);
  layout->addWidget(new ScrollBox(make_scroll_box_body(model)));
  if(!model.m_prerequisites.empty()) {
    layout->addWidget(make_prerequisite_container(model));
  }
  auto tip = new InfoTip(this, parent);
  tip->set_interactive(true);
}

QSize OrderFieldInfoTip::sizeHint() const {
  auto height = [&] {
    auto size = QWidget::sizeHint();
    auto max_height = scale_height(300);
    if(layout()->count() > 1) {
      auto scroll_box_height = layout()->itemAt(0)->sizeHint().height();
      return std::min(scroll_box_height, max_height) +
        size.height() - scroll_box_height;
    }
    return std::min(size.height(), max_height);
  }();
  return QSize(scale_width(300), height);
}
