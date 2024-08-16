#ifndef SPIRE_LABELCELL_HPP
#define SPIRE_LABELCELL_HPP
#include <string>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/CanvasView/CanvasCell.hpp"

namespace Spire {

  /*! \class LabelCell
      \brief A cell that displays a CanvasNode's label.
   */
  class LabelCell : public CanvasCell {
    public:

      //! Constructs a LabelCell.
      /*!
        \param label The cell's label.
        \param node The CanvasNode represented by this cell.
      */
      LabelCell(const std::string& label, Beam::Ref<const CanvasNode> node);

      virtual const CanvasNode& GetNode() const;

      virtual QTableWidgetItem* clone() const;

      virtual QVariant data(int role) const;

    private:
      std::string m_name;
      const CanvasNode* m_node;
  };
}

#endif
