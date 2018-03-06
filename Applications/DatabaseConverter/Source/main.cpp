#include <fstream>
#include <iostream>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include <tclap/CmdLine.h>
#include "DatabaseConverter/Version.hpp"
#include "Nexus/OrderExecutionService/MySqlOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Beam;
using namespace Beam::MySql;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace std;
using namespace TCLAP;

sql_create_2(submission_rows, 2, 0,
  mysqlpp::sql_bigint_unsigned, id,
  mysqlpp::sql_blob, additional_fields);

struct ClassicTag {
  using Type = boost::variant<int, double, std::int64_t, std::int64_t, char,
    std::string, boost::gregorian::date, boost::posix_time::time_duration,
    boost::posix_time::ptime>;
  int m_key;
  Type m_value;
};

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<ClassicTag> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, ClassicTag& value,
        unsigned int version) {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}
}

namespace {
  struct TagVisitor {
    int index;
    vector<Tag>* m_tags;

    void operator ()(const std::int64_t&) const {}

    template<typename T>
    void operator ()(const T& value) const {
      m_tags->emplace_back(index, value);
    }
  };
  void ConvertOrderExecutionDataStore(const MySqlConfig& config) {
    if(config.m_address.GetHost().empty()) {
      return;
    }
    mysqlpp::Connection connection{false};
    auto connectionResult = connection.set_option(
      new mysqlpp::ReconnectOption{true});
    if(!connectionResult) {
      throw std::runtime_error{"Unable to set MySQL reconnect option."};
    }
    connectionResult = connection.connect(config.m_schema.c_str(),
      config.m_address.GetHost().c_str(), config.m_username.c_str(),
      config.m_password.c_str(), config.m_address.GetPort());
    if(!connectionResult) {
      throw std::runtime_error{std::string(
        "Unable to connect to MySQL database - ") + connection.error()};
    }
    OrderId id = 0;
    while(true) {
      mysqlpp::Transaction t{connection};
      auto query = connection.query();
      std::vector<submission_rows> rows;
      try {
        query << "SELECT id, additional_fields FROM submissions WHERE id >= " <<
          id << " LIMIT 10000";
        query.storein(rows);
        for(auto& row : rows) {
          if(!row.additional_fields.empty()) {
            Beam::Serialization::BinaryReceiver<
              Beam::IO::SharedBuffer> receiver;
            Beam::IO::SharedBuffer additionalFieldsBuffer{
              row.additional_fields.data(), row.additional_fields.size()};
            receiver.SetSource(Beam::Ref(additionalFieldsBuffer));
            try {
              vector<ClassicTag> classicTags;
              receiver.Shuttle(classicTags);
              vector<Tag> tags;
              for(auto& classicTag : classicTags) {
                if(classicTag.m_value.which() == Tag::MONEY_INDEX) {
                  auto original = boost::get<int64_t>(classicTag.m_value);
                  auto conversion = Quantity{original};
                  conversion /= Quantity::MULTIPLIER;
                  auto m = Money{conversion};
                  tags.emplace_back(Tag::MONEY_INDEX, m);
                  std::cout << "Money: " << original << " " << conversion << " "
                    << m << std::endl;
                } else if(classicTag.m_value.which() == Tag::QUANTITY_INDEX) {
                  auto original = boost::get<int64_t>(classicTag.m_value);
                  auto conversion = Quantity{original};
                  tags.emplace_back(Tag::QUANTITY_INDEX, conversion);
                  std::cout << "Qty: " << original << " " << conversion <<
                    std::endl;
                } else {
                  boost::apply_visitor(TagVisitor{classicTag.m_key,
                    &tags}, classicTag.m_value);
                }
              }
            } catch(const Beam::Serialization::SerializationException&) {
              BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
                "Unable to load additional fields."));
            }
          }
        }
      } catch(...) {
        t.rollback();
        throw;
      }
      t.commit();
    }
  }

  void ConvertComplianceDataStore(const MySqlConfig& config) {
  }
}

int main(int argc, const char** argv) {
  string configFile;
  string backupFile;
  try {
    CmdLine cmd{"", ' ', "0.9-r" DATABASE_CONVERTER_VERSION
      "\nCopyright (C) 2014 Eidolon Systems Ltd."};
    ValueArg<string> configArg{"c", "config", "Configuration file", false,
      "config.yml", "path"};
    ValueArg<string> backupArg{"b", "backup", "Backup file", true, "", "path"};
    cmd.add(configArg);
    cmd.parse(argc, argv);
    configFile = configArg.getValue();
    backupFile = backupArg.getValue();
  } catch(const ArgException& e) {
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }
  YAML::Node config;
  try {
    std::ifstream configStream{configFile.c_str()};
    if(!configStream.good()) {
      cerr << configFile << " not found." << endl;
      return -1;
    }
    YAML::Parser configParser{configStream};
    configParser.GetNextDocument(config);
  } catch(const YAML::ParserException& e) {
    cerr << "Invalid YAML at line " << (e.mark.line + 1) << ", " << "column " <<
      (e.mark.column + 1) << ": " << e.msg << endl;
    return -1;
  }
  MySqlConfig orderSqlConfig;
  try {
    orderSqlConfig = MySqlConfig::Parse(*config.FindValue("order_data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  ConvertOrderExecutionDataStore(orderSqlConfig);
  MySqlConfig complianceSqlConfig;
  try {
    complianceSqlConfig = MySqlConfig::Parse(*config.FindValue(
      "compliance_data_store"));
  } catch(const std::exception& e) {
    cerr << "Error parsing section 'data_store': " << e.what() << endl;
    return -1;
  }
  ConvertComplianceDataStore(complianceSqlConfig);
  return 0;
}
