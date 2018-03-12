#include <fstream>
#include <iostream>
#include <Beam/MySql/MySqlConfig.hpp>
#include <Beam/Utilities/YamlConfig.hpp>
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
#include <tclap/CmdLine.h>
#include "DatabaseConverter/Version.hpp"
#include "Nexus/Compliance/MySqlComplianceRuleDataStore.hpp"
#include "Nexus/OrderExecutionService/MySqlOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::MySql;
using namespace Beam::Serialization;
using namespace boost;
using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::OrderExecutionService;
using namespace std;
using namespace TCLAP;

sql_create_2(submission_rows, 2, 0,
  mysqlpp::sql_bigint_unsigned, order_id,
  mysqlpp::sql_blob, additional_fields);

sql_create_2(entry_rows, 2, 0,
  mysqlpp::sql_bigint_unsigned, entry_id,
  mysqlpp::sql_blob, schema_parameters);

struct ClassicQuantity {
  std::int64_t m_original;
  Quantity m_value;
};

struct ClassicMoney {
  double m_original;
  Money m_value;
};

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<ClassicQuantity> : std::false_type {};
  template<>
  struct IsStructure<ClassicMoney> : std::false_type {};

  template<>
  struct Receive<ClassicQuantity> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        ClassicQuantity& value) const {
      std::int64_t v;
      shuttle.Shuttle(name, v);
      value.m_original = v;
      value.m_value = Quantity{v};
    }
  };

  template<>
  struct Receive<ClassicMoney> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        ClassicMoney& value) const {
      std::int64_t v;
      shuttle.Shuttle(name, v);
      auto conversion = Quantity{v};
      conversion /= Quantity::MULTIPLIER;
      value.m_original = static_cast<double>(v) / Quantity::MULTIPLIER;
      value.m_value = Money{conversion};
    }
  };
}
}

struct ClassicTag {
  using Type = boost::variant<int, double, ClassicQuantity, ClassicMoney, char,
    std::string, boost::gregorian::date, boost::posix_time::time_duration,
    boost::posix_time::ptime>;
  int m_key;
  Type m_value;
};

struct ClassicParameter {
  using Type = boost::make_recursive_variant<bool, ClassicQuantity, double,
    std::string, boost::posix_time::ptime, boost::posix_time::time_duration,
    CurrencyId, ClassicMoney, Security,
    std::vector<boost::recursive_variant_>>::type;
  std::string m_name;
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

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<ClassicParameter> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, ClassicParameter& value,
        unsigned int version) {
      shuttle.Shuttle("name", value.m_name);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}
}

namespace {
  struct TagVisitor {
    int index;
    vector<Tag>* m_tags;

    void operator ()(ClassicQuantity value) const {
      std::cout << "\tQuantity: " << value.m_original << " " << value.m_value <<
        std::endl;
      m_tags->emplace_back(index, value.m_value);
    }

    void operator ()(ClassicMoney value) const {
      std::cout << "\tMoney: " << value.m_original << " " << value.m_value <<
        std::endl;
      m_tags->emplace_back(index, value.m_value);
    }

    template<typename T>
    void operator ()(T value) const {
      m_tags->emplace_back(index, value);
    }
  };

  struct SubVisitor {
    std::vector<ComplianceValue>* values;

    void operator ()(ClassicQuantity value) const {
      std::cout << "\tQuantity: " << value.m_original << " " << value.m_value <<
        std::endl;
      values->emplace_back(value.m_value);
    }

    void operator ()(ClassicMoney value) const {
      std::cout << "\tMoney: " << value.m_original << " " << value.m_value <<
        std::endl;
      values->emplace_back(value.m_value);
    }

    template<typename T>
    void operator ()(T value) const {
      values->emplace_back(value);
    }

    template<typename T>
    void operator ()(std::vector<T> value) const {
      std::terminate();
    }
  };

  struct ParameterVisitor {
    std::string m_name;
    vector<ComplianceParameter>* m_parameters;

    void operator ()(ClassicQuantity value) const {
      std::cout << "\tQuantity: " << value.m_original << " " << value.m_value <<
        std::endl;
      m_parameters->emplace_back(m_name, value.m_value);
    }

    void operator ()(ClassicMoney value) const {
      std::cout << "\tMoney: " << value.m_original << " " << value.m_value <<
        std::endl;
      m_parameters->emplace_back(m_name, value.m_value);
    }

    template<typename T>
    void operator ()(T value) const {
      m_parameters->push_back(ComplianceParameter(m_name, value));
    }

    template<typename T>
    void operator ()(std::vector<T> value) const {
      std::vector<ComplianceValue> v;
      for(auto& parameter : value) {
        boost::apply_visitor(SubVisitor{&v}, parameter);
      }
      m_parameters->push_back(ComplianceParameter(m_name, v));
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
      std::vector<submission_rows> rows;
      try {
        {
          auto query = connection.query();
          query << "SELECT order_id, additional_fields FROM submissions WHERE "
            "order_id >= " << id << " ORDER BY order_id LIMIT 10000";
          query.storein(rows);
          if(rows.empty()) {
            break;
          }
        }
        for(auto& row : rows) {
          auto query = connection.query();
          id = row.order_id + 1;
          std::cout << "Order: " << (id - 1) << std::endl;
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
                boost::apply_visitor(TagVisitor{classicTag.m_key, &tags},
                  classicTag.m_value);
              }
              SharedBuffer updateBuffer;
              {
                BinarySender<SharedBuffer> sender;
                sender.SetSink(Ref(updateBuffer));
                sender.Shuttle(tags);
              }
              mysqlpp::sql_blob updated_row(updateBuffer.GetData(),
                updateBuffer.GetSize());
              query << "UPDATE submissions SET additional_fields = " <<
                mysqlpp::quote << updated_row << " WHERE order_id = " <<
                row.order_id;
            } catch(const Beam::Serialization::SerializationException&) {
              BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
                "Unable to load additional fields."));
            }
          }
          query.execute();
        }
      } catch(...) {
        t.rollback();
        throw;
      }
      t.commit();
    }
  }

  void ConvertComplianceDataStore(const MySqlConfig& config) {
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
    ComplianceRuleId id = 0;
    while(true) {
      mysqlpp::Transaction t{connection};
      std::vector<entry_rows> rows;
      try {
        {
          auto query = connection.query();
          query << "SELECT entry_id, schema_parameters FROM "
            "compliance_rule_entries WHERE entry_id >= " << id <<
            " ORDER BY entry_id LIMIT 10000";
          query.storein(rows);
          if(rows.empty()) {
            break;
          }
        }
        for(auto& row : rows) {
          auto query = connection.query();
          id = row.entry_id + 1;
          std::cout << "Compliance: " << (id - 1) << std::endl;
          Beam::Serialization::BinaryReceiver<Beam::IO::SharedBuffer> receiver;
          Beam::IO::SharedBuffer buffer{row.schema_parameters.data(),
            row.schema_parameters.size()};
          receiver.SetSource(Beam::Ref(buffer));
          try {
            vector<ClassicParameter> classicParameters;
            receiver.Shuttle(classicParameters);
            vector<ComplianceParameter> parameters;
            for(auto& classicParameter : classicParameters) {
              boost::apply_visitor(ParameterVisitor{classicParameter.m_name,
                &parameters}, classicParameter.m_value);
            }
            SharedBuffer updateBuffer;
            {
              BinarySender<SharedBuffer> sender;
              sender.SetSink(Ref(updateBuffer));
              sender.Shuttle(parameters);
            }
            mysqlpp::sql_blob updated_row(updateBuffer.GetData(),
              updateBuffer.GetSize());
            query << "UPDATE compliance_rule_entries SET "
              "schema_parameters = " << mysqlpp::quote << updated_row <<
              " WHERE entry_id = " << row.entry_id;
          } catch(const Beam::Serialization::SerializationException&) {
            BOOST_THROW_EXCEPTION(OrderExecutionDataStoreException(
              "Unable to load additional fields."));
          }
          query.execute();
        }
      } catch(...) {
        t.rollback();
        throw;
      }
      t.commit();
    }
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
