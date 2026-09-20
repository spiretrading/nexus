#ifndef NEXUS_TO_PYTHON_STAMP_FIELD_HPP
#define NEXUS_TO_PYTHON_STAMP_FIELD_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include "Nexus/Stamp/StampField.hpp"

namespace Nexus {

  /** Owns a STAMP field for use with Python. */
  class ToPythonStampField {
    public:

      /**
       * Copies a STAMP field into owned storage.
       * @param field The field to copy.
       */
      explicit ToPythonStampField(const StampField& field);

      /** Returns the underlying field. */
      const StampField& get() const;

    private:
      Beam::SharedBuffer m_source;
      StampField m_field;
  };

  inline ToPythonStampField::ToPythonStampField(const StampField& field)
    : m_source(field.m_value.data(), field.m_value.size()),
      m_field(field.m_identifier, field.m_index,
        std::string_view(m_source.get_data(), m_source.get_size())) {}

  inline const StampField& ToPythonStampField::get() const {
    return m_field;
  }
}

#endif
