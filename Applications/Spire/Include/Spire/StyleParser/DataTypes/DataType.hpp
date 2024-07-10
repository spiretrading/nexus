#ifndef SPIRE_DATA_TYPE_HPP
#define SPIRE_DATA_TYPE_HPP
#include <string>

namespace Spire {

  /** Represents the abstract type. */
  class DataType {
    public:
      virtual ~DataType() = default;

      /** Returns the name. */
      const std::string& get_name() const;

      bool operator ==(const DataType& rhs) const;

    protected:
      virtual bool is_equal(const DataType& rhs) const = 0;
      
      explicit DataType(std::string name);

    private:
      std::string m_name;
  };
}

#endif
