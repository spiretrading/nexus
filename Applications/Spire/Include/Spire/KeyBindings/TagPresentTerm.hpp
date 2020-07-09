#ifndef SPIRE_TAG_PRESENT_TERM_HPP
#define SPIRE_TAG_PRESENT_TERM_HPP
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  //! Is satisfied iff a list of tags contains a tag with a selected key.
  class TagPresentTerm : public Term {
    public:

      //! Constructs a TagPresentTerm.
      /*!
        \param tag_key The key of the tag that should be present.
      */
      explicit TagPresentTerm(int tag_key);

      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const override;

    private:
      int m_tag_key;
  };
}

#endif
