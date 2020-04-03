#ifndef SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#define SPIRE_CANCEL_KEY_BINDINGS_TABLE_MODEL_HPP
#include <QAbstractTableModel>
#include "Spire/KeyBindings/KeyBindings.hpp"

namespace Spire {

  class CancelKeyBindingsTableModel : public QAbstractTableModel {
    public:

      CancelKeyBindingsTableModel(
        const std::vector<KeyBindings::CancelActionBinding>& bindings);

      void set_key_bindings(
        const std::vector<KeyBindings::CancelActionBinding>& bindings);

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant data(const QModelIndex& index, int role) const override;

      //QVariant headerData(int section, Qt::Orientation orientation,
      //  int role) const override;

    private:
      std::vector<KeyBindings::CancelActionBinding> m_key_bindings;
  };
}

#endif
