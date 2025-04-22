#include <QApplication>
#include <QTimer>
#include "Spire/BookView/AggregateBookViewModel.hpp"
#include "Spire/BookView/BookViewTableView.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

int main(int argc, char** argv) {
  auto application = QApplication(argc, argv);
  application.setOrganizationName(QObject::tr("Spire Trading Inc"));
  application.setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto model = make_local_aggregate_book_view_model();
  auto p = BookViewProperties::get_default();
  p.m_level_properties.m_color_scheme[0] = QColor(0xFF0000);
  while(p.m_level_properties.m_color_scheme.size() > 1) {
    p.m_level_properties.m_color_scheme.pop_back();
  }
  auto properties = std::make_shared<LocalBookViewPropertiesModel>(p);
  auto view = make_book_view_table_view(model, properties, Side::BID);
  model->get_bids()->push(BookQuote(
    "FOO", true, MarketCode("TSX"), Quote(Money::ONE, 100, Side::BID),
    ptime()));
  QTimer::singleShot(2000, [&] {
    model->get_bids()->remove(0);
    QTimer::singleShot(2000, [&] {
      model->get_bids()->push(BookQuote(
        "FOO", true, MarketCode("TSX"), Quote(Money::ONE, 100, Side::BID),
        ptime()));
    });
  });
  view->show();
  application.exec();
}
