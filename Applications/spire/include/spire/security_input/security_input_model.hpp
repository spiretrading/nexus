#ifndef SPIRE_SECURITY_INPUT_MODEL_HPP
#define SPIRE_SECURITY_INPUT_MODEL_HPP
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/SecurityInfo.hpp"
#include "spire/security_input/security_input.hpp"

namespace spire {

  //! Represents a searchable database of available securities.
  class security_input_model : private boost::noncopyable {
    public:
      virtual ~security_input_model() = default;

      //! Autocompletes a partial search string for a security. The search
      //! query can be a company name or ticker symbol.
      virtual std::vector<Nexus::SecurityInfo> autocomplete(
        const std::string& query) = 0;
  };
}

#endif
