#ifndef SPIRE_REGION_TERM_HPP
#define SPIRE_REGION_TERM_HPP
#include <vector>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  /*! \class RegionTerm
      \brief Checks whether a list of tags contains a region tags which value
             is a sub-region of the region provided.
  */
  class RegionTerm : public Term {
    public:

      //! Constructs a RegionTerm.
      /*!
        \param region The region a region tag should be a sub-region of.
      */
      explicit RegionTerm(Nexus::Region region);

      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const override;
  };
}

#endif
