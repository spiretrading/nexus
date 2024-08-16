#ifndef SPIRE_RECORDTYPE_HPP
#define SPIRE_RECORDTYPE_HPP
#include <memory>
#include <vector>
#include <boost/optional/optional.hpp>
#include "Spire/Canvas/Canvas.hpp"
#include "Spire/Canvas/Types/NativeType.hpp"

namespace Spire {

  /*! \class RecordType
      \brief Stores information about a Record data type.
   */
  class RecordType : public NativeType {
    public:

      //! Defines the native type being represented.
      typedef Record Type;

      /*! \struct Field
          \brief Stores a field.
       */
      struct Field {

        //! The name of the Field.
        std::string m_name;

        //! The Field's type.
        std::shared_ptr<NativeType> m_type;

        //! Constructs an empty Field.
        Field() = default;

        //! Copies a Field.
        Field(const Field& field) = default;

        //! Constructs a Field.
        /*!
          \param name The name of the Field.
          \param type The Field's type.
        */
        Field(std::string name, const NativeType& type);

        template<typename Shuttler>
        void Shuttle(Shuttler& shuttle, unsigned int version);
      };

      //! Returns a RecordType containing no Fields.
      static const RecordType& GetEmptyRecordType();

      //! Returns the shared_ptr managing this RecordType.
      operator std::shared_ptr<RecordType> () const;

      //! Returns the Fields.
      const std::vector<Field>& GetFields() const;

      virtual std::string GetName() const;

      virtual const std::type_info& GetNativeType() const;

      virtual Compatibility GetCompatibility(const CanvasType& type) const;

      virtual void Apply(CanvasTypeVisitor& visitor) const;

    private:
      friend std::shared_ptr<RecordType> MakeRecordType(
        std::vector<Field> fields);
      friend std::shared_ptr<RecordType> MakeRecordType(std::string name,
        std::vector<Field> fields);
      friend struct Beam::Serialization::DataShuttle;
      std::string m_name;
      std::vector<Field> m_fields;

      RecordType();
      RecordType(std::vector<Field> fields);
      RecordType(std::string name, std::vector<Field> fields);
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  //! Makes an anonymous RecordType.
  /*!
    \param fields The RecordType's Fields.
  */
  std::shared_ptr<RecordType> MakeRecordType(
    std::vector<RecordType::Field> fields);

  //! Makes a RecordType.
  /*!
    \param name The name of the RecordType.
    \param fields The RecordType's Fields.
  */
  std::shared_ptr<RecordType> MakeRecordType(std::string name,
    std::vector<RecordType::Field> fields);

  //! Finds a Field with a specified name within a RecordType.
  /*!
    \param type The RecordType to find the Field in.
    \param name The name of the Field to find.
    \return The Field with the specified name.
  */
  boost::optional<const RecordType::Field&> FindField(const RecordType& type,
    const std::string& name);

  template<typename Shuttler>
  void RecordType::Field::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("type", m_type);
  }

  template<typename Shuttler>
  void RecordType::Shuttle(Shuttler& shuttle, unsigned int version) {
    NativeType::Shuttle(shuttle, version);
    shuttle.Shuttle("name", m_name);
    shuttle.Shuttle("fields", m_fields);
  }
}

#endif
