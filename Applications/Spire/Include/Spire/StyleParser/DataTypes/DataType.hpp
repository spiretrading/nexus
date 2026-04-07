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

      /**
       * Constructs a DataType.
       * @param name The name of the DataType.
       */
      explicit DataType(std::string name);

      virtual bool is_equal(const DataType& rhs) const = 0;

    private:
      std::string m_name;
  };
}

#endif
