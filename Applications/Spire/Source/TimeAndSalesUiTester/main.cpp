#include <QApplication>
#include <QFontDatabase>
#include <QTextEdit>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"
#include "Spire/TimeAndSales/NoneTimeAndSalesModel.hpp"
#include "Spire/TimeAndSales/TimeAndSalesWindow.hpp"
#include "Spire/TimeAndSales/TimeAndSalesPropertiesWindow.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Version.hpp"

using namespace Nexus;
using namespace Spire;

std::shared_ptr<ComboBox::QueryModel> populate_securities() {
  auto security_infos = std::vector<SecurityInfo>();
  security_infos.emplace_back(ParseSecurity("MRU.TSX"),
    "Metro Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MG.TSX"),
    "Magna International Inc.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGA.TSX"),
    "Mega Uranium Ltd.", "", 0);
  security_infos.emplace_back(ParseSecurity("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF", "", 0);
  security_infos.emplace_back(ParseSecurity("MON.NYSE"),
    "Monsanto Co.", "", 0);
  security_infos.emplace_back(ParseSecurity("MFC.TSX"),
    "Manulife Financial Corporation", "", 0);
  security_infos.emplace_back(ParseSecurity("MX.TSX"),
    "Methanex Corporation", "", 0);
  auto model = std::make_shared<LocalComboBoxQueryModel>();
  for(auto security_info : security_infos) {
    model->add(to_text(security_info.m_security).toLower(), security_info);
    model->add(
      QString::fromStdString(security_info.m_name).toLower(), security_info);
  }
  return model;
}

std::shared_ptr<TimeAndSalesModel> model_builder(const Security& security) {
  return std::make_shared<NoneTimeAndSalesModel>();
}

QString to_text(const TimeAndSalesProperties& properties,
    const TimeAndSalesProperties& previous_properties) {
  auto text = QString();
  if(properties.get_font() != previous_properties.get_font()) {
    auto& font = properties.get_font();
    text += QString("Font: %1, %2, %3").
      arg(font.family()).
      arg(QFontDatabase().styleString(font)).
      arg(unscale_width(font.pixelSize()));
  }
  if(properties.is_grid_enabled() != previous_properties.is_grid_enabled()) {
    if(!text.isEmpty()) {
      text += "\n";
    }
    text += QString("Grid enabled: %1").arg(properties.is_grid_enabled());
  }
  for(auto i = 0; i < BBO_INDICATOR_COUNT; ++i) {
    auto indicator = static_cast<BboIndicator>(i);
    if(properties.get_highlight_color(indicator) !=
        previous_properties.get_highlight_color(indicator)) {
      if(!text.isEmpty()) {
        text += "\n";
      }
      auto& highlight = properties.get_highlight_color(indicator);
      text += QString("Highlight of the Indicator %1: [%2 %3]").arg(i).
        arg(highlight.m_background_color.name()).
        arg(highlight.m_text_color.name());
    }
  }
  return text;
}

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Time and Sales UI Tester"));
  initialize_resources();
  auto factory = std::make_shared<TimeAndSalesPropertiesWindowFactory>();
  auto time_and_sales_window = TimeAndSalesWindow(populate_securities(),
    factory, &model_builder);
  time_and_sales_window.show();
  auto properties_window = factory->make();
  properties_window->show();
  auto previous_properties = properties_window->get_current()->get();
  auto output_widget = QWidget();
  auto output = new QTextEdit();
  output->setReadOnly(true);
  enclose(output_widget, *output);
  properties_window->get_current()->connect_update_signal(
    [&] (const auto& properties) {
      if(auto text = to_text(properties, previous_properties);
          !text.isEmpty()) {
        output->append(text);
      }
      previous_properties = properties;
    });
  output_widget.show();
  output_widget.resize(scale_width(300), properties_window->height());
  output_widget.move(
    properties_window->pos().x() + properties_window->width() + 1,
    properties_window->pos().y());
  application.exec();
}
