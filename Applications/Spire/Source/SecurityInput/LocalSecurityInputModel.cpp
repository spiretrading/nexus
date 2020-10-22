#include "Spire/SecurityInput/LocalSecurityInputModel.hpp"
#include <unordered_set>
#include <boost/algorithm/string/case_conv.hpp>

using namespace Beam;
using namespace Nexus;
using namespace Spire;

LocalSecurityInputModel::LocalSecurityInputModel()
    : m_securities('\0') {}

void LocalSecurityInputModel::add(SecurityInfo security) {
  auto key = ToString(security.m_security, GetDefaultMarketDatabase());
  auto name = boost::to_upper_copy(security.m_name);
  m_securities[key.c_str()] = security;
  m_securities[name.c_str()] = std::move(security);
}

QtPromise<std::vector<SecurityInfo>> LocalSecurityInputModel::autocomplete(
    const std::string& query) {
  constexpr auto MAX_MATCH_COUNT = std::size_t{8};
  auto matches = [&] {
    if(query.empty()) {
      return std::vector<SecurityInfo>();
    }
    auto matches = std::unordered_set<SecurityInfo>();
    auto uppercasePrefix = boost::to_upper_copy(query);
    for(auto i = m_securities.startsWith(uppercasePrefix.c_str());
        i != m_securities.end(); ++i) {
      matches.insert(*i->second);
      if(matches.size() >= MAX_MATCH_COUNT) {
        break;
      }
    }
    return std::vector<SecurityInfo>(matches.begin(), matches.end());
  }();
  return QtPromise(
    [matches = std::move(matches)] {
      return std::move(matches);
    });
}

Ref<LocalSecurityInputModel> Spire::get_local_security_input_test_model() {
  static LocalSecurityInputModel& model = [] () -> LocalSecurityInputModel& {
    auto model = new LocalSecurityInputModel();
    model->add(SecurityInfo(
      Security("MSFT", DefaultMarkets::NASDAQ(), DefaultCountries::US()),
      "Microsoft Corp", "Software", 100));
    model->add(SecurityInfo(
      Security("MG", DefaultMarkets::TSX(), DefaultCountries::CA()),
      "Magna International Inc.", "Automotive", 100));
    model->add(SecurityInfo(
      Security("MFC", DefaultMarkets::TSX(), DefaultCountries::CA()),
      "Manulife Financial Corporation", "Finance", 100));
    model->add(SecurityInfo(
      Security("MX", DefaultMarkets::TSX(), DefaultCountries::CA()),
      "Methanex Corporation", "", 100));
    model->add(SecurityInfo(
      Security("MRU", DefaultMarkets::TSX(), DefaultCountries::CA()),
      "Metro Inc.", "", 100));
    model->add(SecurityInfo(
      Security("MON", DefaultMarkets::NYSE(), DefaultCountries::US()),
      "Monsanto Co.", "", 100));
    model->add(SecurityInfo(
      Security("MS", DefaultMarkets::NYSE(), DefaultCountries::US()),
      "Morgan Stanley", "Finance", 100));
    return *model;
  }();
  return Ref(model);
}
