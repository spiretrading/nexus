#include <QApplication>
#include <QGridLayout>
#include "Nexus/Definitions/SecuritySet.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/PopupBox.hpp"
#include "Spire/Ui/TagBox.hpp"
#include "Spire/Ui/TagComboBox.hpp"
#include "Spire/Ui/TextBox.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

std::shared_ptr<ArrayListModel<QString>> populate_tag_box_model() {
  auto model = std::make_shared<ArrayListModel<QString>>();
  model->push("CAN");
  model->push("MSFT.NSDQ");
  model->push("XIU.TSX");
  model->push("TSX");
  model->push("TSN.TSX");
  model->push("TSO.ASX");
  return model;
}

std::shared_ptr<LocalComboBoxQueryModel> populate_tag_combo_box_model() {
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  model->add(QString("TSX"));
  model->add(QString("TSXV"));
  model->add(QString("TSO.ASX"));
  model->add(QString("TSU.TSX"));
  model->add(QString("TSN.TSXV"));
  model->add(QString("TSL.NYSE"));
  model->add(QString("MSFT.NSDQ"));
  model->add(QString("XDRX"));
  model->add(QString("XIU.TSX"));
  model->add(QString("AUS"));
  model->add(QString("CAN"));
  model->add(QString("CHN"));
  model->add(QString("JPN"));
  model->add(QString("USA"));
  return model;
}

class DemoWidget : public QWidget {
  public:
    explicit DemoWidget(QWidget* parent = nullptr)
        : QWidget(parent) {
      auto internal_layout = new QGridLayout();
      internal_layout->setSpacing(0);
      for(auto i = 0; i < 5; ++i) {
        for(auto j = 0; j < 3; ++j) {
          auto widget = [&] () -> QWidget* {
            if(i == 3 && j == 1) {
              auto tag_combo_box =
                new TagComboBox(populate_tag_combo_box_model());
              tag_combo_box->set_placeholder("TagComboBox");
              update_style(*tag_combo_box, [&] (auto& style) {
                style.get(Any() > is_a<TagBox>()).set(TagBoxOverflow::ELIDE);
              });
              return new PopupBox(*tag_combo_box);
            } else if(i == 4 && j == 1) {
              auto tag_box = new TagBox(populate_tag_box_model(),
                std::make_shared<LocalTextModel>());
              tag_box->set_placeholder("TagBox");
              update_style(*tag_box, [&] (auto& style) {
                style.get(Any()).set(TagBoxOverflow::ELIDE);
              });
              tag_box->connect_submit_signal([=] (const auto& value) {
                if(!value.isEmpty()) {
                  tag_box->get_tags()->push(value);
                }
              });
              return new PopupBox(*tag_box);
            }
            auto text_box = new TextBox(QString("%1").arg(i));
            return new PopupBox(*text_box);
          }();
          internal_layout->addWidget(widget, i, j);
          if(j != 0) {
            internal_layout->setColumnStretch(j, 1);
          }
          internal_layout->setRowMinimumHeight(i, widget->sizeHint().height());
        }
      }
      auto layout = make_vbox_layout(this);
      layout->addLayout(internal_layout);
      layout->addStretch(1);
    }
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto widget = DemoWidget();
  widget.show();
  widget.resize(scale(500, 200));
  application->exec();
}
