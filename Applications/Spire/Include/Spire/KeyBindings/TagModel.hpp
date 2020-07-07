#ifndef SPIRE_TAG_MODEL_HPP
#define SPIRE_TAG_MODEL_HPP
#include <string>
#include <vector>
#include "Spire/KeyBindings/KeyBindings.hpp"
#include "Spire/KeyBindings/Term.hpp"

namespace Spire {

	/*! \class TagModel
	    \brief Stores key binding tags with their requirements. 
	*/
	class TagModel {

		/*! \struct Schema
		    \brief Encapsulates a tag along with its display name and requirements.
		*/
		struct Schema {

			//! The display name of the tag.
			std::string m_name;

			//! The tag.
			KeyBindings::Tag m_tag;

			//! A term evaluating the requirements for the tag to be enabled.
			Term m_requirement;
		};

		//! Constructs a TagModel.
		/*!
		  \param schemas A list of Schemas of the tags to store.
		*/
		explicit TagModel(std::vector<Schema> schemas);

		//! Returns the display name of the tag with a given key.
		/*!
		  \param tag_key The key of the tag.
		*/
		std::string get_tag_name(int tag_key) const;

		//! Returns a list of tags enabled for a set of prior tags.
		/*!
		  \param prior_tags A list of already set tags.
		*/
		std::vector<KeyBindings::Tag> load_enabled_tags(
			const std::vector<KeyBindings::Tag>& prior_tags) const;
	};
}

#endif
