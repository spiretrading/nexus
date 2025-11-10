#ifndef SPIRE_DASHBOARDMODELSCHEMA_HPP
#define SPIRE_DASHBOARDMODELSCHEMA_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include <Beam/Serialization/DataShuttle.hpp>
#include <Beam/Serialization/ShuttleVariant.hpp>
#include <Beam/Serialization/ShuttleVector.hpp>
#include "Spire/Dashboard/Dashboard.hpp"
#include "Spire/Dashboard/DashboardCell.hpp"
#include "Spire/Dashboard/DashboardRowBuilder.hpp"

namespace Spire {

  /*! \class DashboardModelSchema
      \brief Stores the information needed to reconstruct a DashboardModel.
   */
  class DashboardModelSchema {
    public:

      //! Returns a DashboardModelSchema that can be used to build a basic
      //! DashboardModel.
      static DashboardModelSchema GetDefaultSchema();

      //! Constructs an empty DashboardModelSchema.
      DashboardModelSchema();

      //! Constructs a DashboardModelSchema.
      /*!
        \param columnNames The list of column names.
        \param rowIndices The list of row indices.
        \param rowBuilder Constructs the rows of the DashboardModel.
      */
      DashboardModelSchema(std::vector<std::string> columnNames,
        std::vector<DashboardCell::Value> rowIndices,
        const DashboardRowBuilder& rowBuilder);

      //! Constructs a DashboardModelSchema from a DashboardModel and
      //! DashboardRowBuilder.
      /*!
        \param model The model to represent.
        \param rowBuilder Constructs the rows of the DashboardModel.
      */
      DashboardModelSchema(const DashboardModel& model,
        const DashboardRowBuilder& rowBuilder);

      //! Copies a DashboardModelSchema.
      /*!
        \param schema The DashboardModelSchema to copy.
      */
      DashboardModelSchema(const DashboardModelSchema& schema);

      ~DashboardModelSchema();

      //! Assigns a DashboardModelSchema.
      /*!
        \param schema The DashboardModelSchema to copy.
        \return A reference to <i>*this</i>.
      */
      DashboardModelSchema& operator =(const DashboardModelSchema& schema);

      //! Returns the list of column names.
      const std::vector<std::string>& GetColumnNames() const;

      //! Returns the list of row indices.
      const std::vector<DashboardCell::Value>& GetRowIndices() const;

      //! Returns the DashboardRowBuilder.
      const DashboardRowBuilder& GetRowBuilder() const;

      //! Constructs a new DashboardModel represented by this schema.
      /*!
        \param userProfile The user's profile.
      */
      std::unique_ptr<DashboardModel> Make(
        Beam::Ref<UserProfile> userProfile) const;

    private:
      friend struct Beam::Shuttle<DashboardModelSchema>;
      std::vector<std::string> m_columnNames;
      std::vector<DashboardCell::Value> m_rowIndices;
      std::unique_ptr<DashboardRowBuilder> m_rowBuilder;
  };
}

namespace Beam {
  template<>
  struct Shuttle<Spire::DashboardModelSchema> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Spire::DashboardModelSchema& value,
        unsigned int version) const {
      shuttle.shuttle("column_names", value.m_columnNames);
      shuttle.shuttle("row_indices", value.m_rowIndices);
      shuttle.shuttle("row_builder", value.m_rowBuilder);
    }
  };
}

#endif
