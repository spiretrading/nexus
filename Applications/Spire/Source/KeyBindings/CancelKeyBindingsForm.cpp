#include "Spire/KeyBindings/CancelKeyBindingsForm.hpp"
#include "Spire/KeyBindings/CancelKeyBindingsModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  using BoldText = StateSelector<void, struct BoldTextSelectorTag>;

  const QString& displayText(CancelKeyBindingsModel::Operation operation) {
    if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT) {
      static const auto value = QObject::tr("Most Recent");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_ASK) {
      static const auto value = QObject::tr("Most Recent Ask");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::MOST_RECENT_BID) {
      static const auto value = QObject::tr("Most Recent Bid");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::OLDEST) {
      static const auto value = QObject::tr("Oldest");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::OLDEST_ASK) {
      static const auto value = QObject::tr("Oldest Ask");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::OLDEST_BID) {
      static const auto value = QObject::tr("Oldest Bid");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::ALL) {
      static const auto value = QObject::tr("All");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::ALL_ASKS) {
      static const auto value = QObject::tr("All Asks");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::ALL_BIDS) {
      static const auto value = QObject::tr("All Bids");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_ASK) {
      static const auto value = QObject::tr("Closest Ask");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::CLOSEST_BID) {
      static const auto value = QObject::tr("Closest Bid");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::FURTHEST_ASK) {
      static const auto value = QObject::tr("Furthest Ask");
      return value;
    } else if(operation == CancelKeyBindingsModel::Operation::FURTHEST_BID) {
      static const auto value = QObject::tr("Furthest Bid");
      return value;
    } else {
      static const auto value = QObject::tr("None");
      return value;
    }
  }

  auto make_help_text_region() {
    auto help_text_body = new QWidget();
    auto help_text_layout = make_hbox_layout(help_text_body);
    auto dash = QString(0x2013);
    help_text_layout->addWidget(make_label(QObject::tr("Allowed keys are: ")));
    auto label1 = make_label("ESC, F1–F12");
    label1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    match(*label1, BoldText());
    help_text_layout->addWidget(label1);
    help_text_layout->addWidget(make_label(QObject::tr(" and ")));
    auto label2 = make_label("0-9");
    label2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    match(*label2, BoldText());
    help_text_layout->addWidget(label2);
    help_text_layout->addWidget(make_label(QObject::tr(" and any combination with ")));
    auto label3 = make_label(QObject::tr("Ctrl, Shift or Alt"));
    label3->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    match(*label3, BoldText());
    help_text_layout->addWidget(label3);
    auto help_text_box = new Box(help_text_body);
    help_text_box->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Preferred);
    update_style(*help_text_box, [] (auto& style) {
      style.get(Any()).
        set(BackgroundColor(QColor(0xFFFFFF))).
        set(horizontal_padding(scale_width(8))).
        set(PaddingTop(scale_height(10))).
        set(PaddingBottom(scale_height(5)));
      auto font = QFont("Roboto");
      font.setWeight(QFont::Normal);
      font.setPixelSize(scale_width(10));
      style.get(Any() > is_a<TextBox>()).
        set(text_style(font, QColor(0x808080)));
      font.setWeight(QFont::Bold);
      style.get(Any() > BoldText()).
        set(Font(font));
    });
    return help_text_box;
  }

  auto make_key_binding_field(CancelKeyBindingsModel::Operation operation,
      const std::shared_ptr<CancelKeyBindingsModel>& model) {
    auto label = make_label(displayText(operation));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto key_input_box = new KeyInputBox(model->get_binding(operation));
    //key_input_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    key_input_box->setFixedWidth(scale_width(164));
    auto widget = new QWidget();
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto layout = make_hbox_layout(widget);
    layout->setSpacing(scale_width(18));
    layout->addWidget(label);
    layout->addWidget(key_input_box);
    return widget;
  }

}

CancelKeyBindingsForm::CancelKeyBindingsForm(
    std::shared_ptr<CancelKeyBindingsModel> bindings, QWidget* parent)
    : QWidget(parent),
      m_bindings(std::move(bindings)) {
  auto cancel_keys_list_body = new QWidget();
  cancel_keys_list_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto list_layout = make_vbox_layout(cancel_keys_list_body);
  list_layout->setSpacing(scale_height(4));
  for(auto i = 0; i < 13; ++i) {
    list_layout->addWidget(make_key_binding_field(
      static_cast<CancelKeyBindingsModel::Operation>(i), m_bindings));
  }
  auto cancel_keys_list = new Box(cancel_keys_list_body);
  update_style(*cancel_keys_list, [] (auto& style) {
    style.get(Any()).
      set(horizontal_padding(scale_width(8))).
      set(vertical_padding(scale_height(8)));
  });
  auto content = new QWidget();
  content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  content->setMinimumWidth(scale_height(384));
  content->setMaximumWidth(scale_height(480));
  auto content_layout = make_vbox_layout(content);
  content_layout->addWidget(make_help_text_region());
  content_layout->addWidget(cancel_keys_list);
  auto body = new QWidget();
  auto body_layout = make_vbox_layout(body);
  body_layout->addWidget(content, 0, Qt::AlignHCenter);
  body_layout->addStretch(1);
  auto scroll_box = new ScrollBox(body);
  scroll_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update_style(*scroll_box, [] (auto& style) {
    style.get(Any()).set(BackgroundColor(QColor(0xFFFFFF)));
  });
  enclose(*this, *scroll_box);
}

const std::shared_ptr<CancelKeyBindingsModel>&
    CancelKeyBindingsForm::get_bindings() const {
  return m_bindings;
}
