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
    font.setWeight(65);
    font.setPixelSize(scale_width(12));
    style.get(ReadOnly()).
      set(BackgroundColor(QColor(Qt::transparent))).
      set(Font(font)).
      set(TextColor(QColor(0x808080))).
      set(horizontal_padding(0));
    return style;
  }

  auto PREREQUISITES_STYLE() {
    auto style = StyleSheet();
    auto font = QFont("Roboto");
    font.setPixelSize(scale_width(10));
    font.setItalic(true);
    style.get(ReadOnly()).
      set(border_color(QColor(Qt::transparent))).
      set(Font(font)).
      set(TextColor(QColor(0x4B23A0))).
      set(LineHeight(scale_height(1.13))).
      set(PaddingLeft(scale_width(20))).
      set(PaddingTop(scale_height(4)));
    return style;
  }

  auto make_description_layout(const OrderFieldInfoTip::Model& model) {
    auto name_label = make_label(QString::fromStdString(model.m_tag.m_name));
    name_label->set_read_only(true);
    set_style(*name_label, NAME_STYLE());
    auto description_label =
      new TextAreaBox(QString::fromStdString(model.m_tag.m_description));
    description_label->set_read_only(true);
    update_style(*description_label, [&] (auto& style) {
      style.get(ReadOnly()).
        set(PaddingTop(scale_height(6))).
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
    update_style(*id_label, [&] (auto& style) {
      style.get(ReadOnly()).set(FontSize(scale_height(10)));
    });
    auto description_label =
      new TextAreaBox(QString::fromStdString(argument.m_description));
    description_label->set_read_only(true);
    description_label->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*description_label, [&] (auto& style) {
      style.get(ReadOnly()).
        set(border_size(0)).
        set(vertical_padding(0)).
        set(FontSize(scale_height(10))).
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
    layout->setVerticalSpacing(scale_height(8));
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

  auto make_scroll_box(const OrderFieldInfoTip::Model& model) {
    auto scroll_box = new ScrollBox(make_scroll_box_body(model));
    scroll_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    scroll_box->setFixedWidth(scale_width(280));
    scroll_box->setMaximumHeight(scale_height(240));
    return scroll_box;
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
    auto header = make_label(QObject::tr("Prerequisites"));
    update_style(*header, [&] (auto& style) {
      style.get(ReadOnly()).set(FontSize(scale_height(10)));
    });
    auto prerequisites_label = new TextAreaBox(describe_prerequisites(model));
    prerequisites_label->set_read_only(true);
    set_style(*prerequisites_label, PREREQUISITES_STYLE());
    auto container = new QWidget();
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_vbox_layout(container);
    layout->addWidget(header);
    layout->addWidget(prerequisites_label);
    auto box = new Box(container);
    box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    update_style(*box, [&] (auto& style) {
      style.get(Any()).
        set(BorderTopColor(QColor(0xE0E0E0))).
        set(BorderTopSize(scale_height(1))).
        set(padding(scale_width(18)));
    });
    return box;
  }
}

OrderFieldInfoTip::OrderFieldInfoTip(Model model, QWidget* parent)
    : QWidget(parent) {
  setFixedWidth(scale_width(480));
  auto layout = make_vbox_layout(this);
  layout->addWidget(make_scroll_box(model));
  if(!model.m_prerequisites.empty()) {
    layout->addWidget(make_prerequisite_container(model));
  }
  auto tip = new InfoTip(this, parent);
  tip->set_interactive(true);
}
