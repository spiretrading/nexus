#include "spire/security_input/local_security_input_model.hpp"
#include <unordered_set>
#include <boost/algorithm/string/case_conv.hpp>

using namespace Nexus;
using namespace spire;

local_security_input_model::local_security_input_model()
    : m_securities('\0') {}

void local_security_input_model::add(SecurityInfo security) {
  auto key = ToString(security.m_security, GetDefaultMarketDatabase());
  auto name = boost::to_upper_copy(security.m_name);
  m_securities[key.c_str()] = security;
  m_securities[name.c_str()] = std::move(security);
}

std::vector<SecurityInfo> local_security_input_model::autocomplete(
    const std::string& query) {
  if(query.empty()) {
    return {};
  }
  static const auto MAX_MATCH_COUNT = 8;
  std::unordered_set<SecurityInfo> matches;
  auto uppercasePrefix = boost::to_upper_copy(query);
  for(auto i = m_securities.startsWith(uppercasePrefix.c_str());
      i != m_securities.end(); ++i) {
    matches.insert(*i->second);
    if(matches.size() >= MAX_MATCH_COUNT) {
      break;
    }
  }
  return std::vector<SecurityInfo>(matches.begin(), matches.end());
}
