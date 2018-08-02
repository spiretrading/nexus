#ifndef SPIRE_EXPORT_MODEL_HPP
#define SPIRE_EXPORT_MODEL_HPP
#include <QAbstractItemModel>
#include <ostream>

namespace Spire {

  //! Exports a model in CSV format.
  /*!
    \param model The model to export.
    \param out The stream to output the model in CSV format.
  */
  void export_model_as_csv(const QAbstractItemModel& model, std::ostream& out);
}

#endif
