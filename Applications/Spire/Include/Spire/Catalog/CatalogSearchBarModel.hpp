#ifndef SPIRE_CATALOGSEARCHBARMODEL_HPP
#define SPIRE_CATALOGSEARCHBARMODEL_HPP
#include <string>
#include <boost/signals2/signal.hpp>
#include "Spire/Catalog/Catalog.hpp"

namespace Spire {

  /*! \class CatalogSearchBarModel
      \brief Models the contents of a search bar displayed in a CatalogTabView.
   */
  class CatalogSearchBarModel {
    public:

      /*! \enum Mode
          \brief The search bar's display mode.
       */
      enum Mode {

        //! The search bar is displaying a label/instruction.
        LABEL,

        //! The search bar is displaying a CatalogEntry's name.
        CATALOG_ENTRY,

        //! The search bar is displaying user provided input.
        INPUT,
      };

      //! Signals an update to the model.
      /*!
        \param mode The model's current Mode.
        \param text The current text.
      */
      typedef boost::signals2::signal<void (Mode mode, const std::string& text)>
        UpdateSignal;

      //! Constructs a CatalogSearchBarModel.
      CatalogSearchBarModel();

      //! Returns the display mode.
      Mode GetMode() const;

      //! Returns the contents of the search bar.
      const std::string& GetText() const;

      //! Sets the search bar to display a list of CatalogEntry names.
      /*!
        \param entries The list of CatalogEntry names to display.
      */
      void SetCatalogEntries(const std::vector<const CatalogEntry*>& entry);

      //! Sets the search bar to display a label.
      /*!
        \param label The label to display.
      */
      void SetLabel(const std::string& label);

      //! Sets the search bar to display user provided input.
      /*!
        \param input The user's input.
      */
      void SetInput(const std::string& input);

      //! Connects a slot to the UpdateSignal.
      /*!
        \param slot The slot to connect.
        \return A connection to the UpdateSignal.
      */
      boost::signals2::connection ConnectUpdateSignal(
        const UpdateSignal::slot_type& slot) const;

    private:
      Mode m_mode;
      std::string m_text;
      mutable UpdateSignal m_updateSignal;
  };
}

#endif
