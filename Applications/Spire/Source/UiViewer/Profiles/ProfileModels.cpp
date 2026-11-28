#include "Spire/UiViewer/Profiles/ProfileModels.hpp"
#include "Nexus/Definitions/StandardCountries.hpp"
#include "Nexus/Definitions/StandardVenues.hpp"
#include "Spire/Spire/ValidatedValueModel.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace Nexus;
using namespace Spire;

std::shared_ptr<TickerInfoQueryModel> Spire::populate_ticker_query_model() {
  auto ticker_infos = std::vector<TickerInfo>();
  auto add_ticker = [&] (const Ticker& ticker,
      const std::string& name) {
    if(ticker) {
      ticker_infos.emplace_back(ticker, name, "", 0);
    }
  };
  add_ticker(parse_ticker("MRU.TSX"), "Metro Inc.");
  add_ticker(parse_ticker("MG.TSX"), "Magna International Inc.");
  add_ticker(parse_ticker("MGA.TSX"), "Mega Uranium Ltd.");
  add_ticker(parse_ticker("MGAB.TSX"),
    "Mackenzie Global Fixed Income Alloc ETF");
  add_ticker(parse_ticker("MON.NYSE"), "Monsanto Co.");
  add_ticker(parse_ticker("MFC.TSX"), "Manulife Financial Corporation");
  add_ticker(parse_ticker("MX.TSX"), "Methanex Corporation");
  auto model = std::make_shared<LocalQueryModel<TickerInfo>>();
  for(auto& ticker_info : ticker_infos) {
    model->add(to_text(ticker_info.m_ticker).toLower(), ticker_info);
    model->add(
      QString::fromStdString(ticker_info.m_name).toLower(), ticker_info);
  }
  return model;
}

std::shared_ptr<LocalQueryModel<QString>>
    Spire::populate_tag_combo_box_model() {
  auto model = std::make_shared<LocalQueryModel<QString>>();
  model->add("TSX");
  model->add("TSXV");
  model->add("TSO.ASX");
  model->add("TSU.TSX");
  model->add("TSN.TSXV");
  model->add("TSL.NYSE");
  model->add("MSFT.NSDQ");
  model->add("XDRX");
  model->add("XIU.TSX");
  model->add("AUS");
  model->add("CAN");
  model->add("CHN");
  model->add("JPN");
  model->add("USA");
  return model;
}

std::shared_ptr<LocalQueryModel<Nexus::Scope>>
    Spire::populate_scope_box_model() {
  auto tickers = std::vector<std::pair<std::string, std::string>>{
    {"MSFT.NSDQ", "Microsoft Corporation"},
    {"MG.TSX", "Magna International Inc."},
    {"MRU.TSX", "Metro Inc."},
    {"MFC.TSX", "Manulife Financial Corporation"},
    {"MX.TSX", "Methanex Corporation"},
    {"TSO.ASX", "Tesoro Resources Limited"}};
  auto venues = std::vector{
    Venues::ASX, Venues::CXD, Venues::CSE, Venues::TSX, Venues::TSXV};
  auto countries = std::vector{Countries::US, Countries::CA, Countries::AU,
    Countries::JP, Countries::CN};
  auto model = std::make_shared<LocalQueryModel<Scope>>();
  for(auto& ticker_info : tickers) {
    auto ticker = parse_ticker(ticker_info.first);
    if(!ticker) {
      continue;
    }
    auto scope = Scope(ticker_info.second);
    scope += ticker;
    model->add(to_text(ticker).toLower(), scope);
    model->add(QString::fromStdString(scope.get_name()).toLower(), scope);
  }
  for(auto& venue : venues) {
    auto entry = VENUES.from(venue);
    auto scope = Scope(entry.m_description);
    scope += venue;
    model->add(to_text(venue).toLower(), scope);
    model->add(QString::fromStdString(scope.get_name()).toLower(), scope);
  }
  for(auto& country : countries) {
    auto scope = Scope(COUNTRIES.from(country).m_name);
    scope += country;
    model->add(to_text(country).toLower(), scope);
    model->add(QString::fromStdString(scope.get_name()).toLower(), scope);
  }
  return model;
}

QValidator::State Spire::key_input_box_validator(const QKeySequence& sequence) {
  if(sequence.count() == 0) {
    return QValidator::Intermediate;
  } else if(sequence.count() > 1) {
    return QValidator::Invalid;
  }
  auto key = sequence[0];
  key &= ~Qt::ShiftModifier;
  key &= ~Qt::ControlModifier;
  key &= ~Qt::AltModifier;
  key &= ~Qt::MetaModifier;
  key &= ~Qt::KeypadModifier;
  key &= ~Qt::GroupSwitchModifier;
  if(key >= Qt::Key_F1 && key <= Qt::Key_F32) {
    return QValidator::Acceptable;
  }
  return QValidator::Invalid;
}

std::shared_ptr<KeySequenceValueModel>
    Spire::populate_key_input_box_model(const QKeySequence& key) {
  auto model = make_validated_value_model(&key_input_box_validator,
    std::make_shared<LocalKeySequenceValueModel>(key));
  return model;
}
