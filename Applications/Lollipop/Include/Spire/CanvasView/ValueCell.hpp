#ifndef SPIRE_VALUECELL_HPP
#define SPIRE_VALUECELL_HPP
#include <vector>
#include <Beam/Pointers/Ref.hpp>
#include "Spire/Canvas/Operations/CanvasNodeValidationError.hpp"
#include "Spire/CanvasView/CanvasCell.hpp"
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class ValueCell
      \brief A cell that displays a CanvasNode's value.
   */
  class ValueCell : public CanvasCell {
    public:

      //! Constructs a ValueCell.
      /*!
        \param node The CanvasNode represented.
      */
      ValueCell(Beam::Ref<const CanvasNode> node);

      //! Returns the errors associated with this cell.
      const std::vector<CanvasNodeValidationError>& GetErrors() const;

      //! Adds an error to be associated with this cell.
      /*!
        \param error The error associated with this cell.
      */
      void AddError(const CanvasNodeValidationError& error);

      //! Resets the error associated with this cell.
      void ResetErrors();

      virtual const CanvasNode& GetNode() const;

      virtual QTableWidgetItem* clone() const;

      virtual QVariant data(int role) const;

    private:
      const CanvasNode* m_node;
      std::vector<CanvasNodeValidationError> m_errors;

      ValueCell(const ValueCell& cell);
  };
}

#endif
