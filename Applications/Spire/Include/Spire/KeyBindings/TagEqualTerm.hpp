#ifndef SPIRE_TAG_EQUAL_TERM_HPP
#define SPIRE_TAG_EQUAL_TERM_HPP
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

  /*! \class TagEqualTerm
      \brief Checks whether the value of a tag in a list is equal to the value
             provided.
  */
  class TagEqualTerm : public Term {
    public:

      //! Constructs a TagEqualTerm.
      /*!
        \param tag The tag to check the value of.
      */
      explicit TagEqualTerm(const KeyBindings::Tag& tag);

      virtual bool is_satisfied(const std::vector<KeyBindings::Tag>&
        tags) const override;
  };
}

#endif
