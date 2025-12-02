#ifndef SPIRE_EXPORTMODEL_HPP
#define SPIRE_EXPORTMODEL_HPP
#include <ostream>
#include "Spire/UI/UI.hpp"

class QAbstractItemModel;

namespace Spire {

  //! Exports a model to an output stream in CSV format.
  /*!
    \param userProfile The user's profile.
    \param model The model to export.
    \param out The output stream to write to.
  */
  void ExportModelAsCsv(UserProfile& userProfile,
    const QAbstractItemModel& model, std::ostream& out);
}

#endif
