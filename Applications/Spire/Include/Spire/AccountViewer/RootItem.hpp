#ifndef SPIRE_ROOTITEM_HPP
#define SPIRE_ROOTITEM_HPP
#include "Spire/AccountViewer/AccountViewItem.hpp"

namespace Spire {

  /*! \class RootItem
      \brief Represents one of the root directories in an AccountViewModel.
   */
  class RootItem : public AccountViewItem {
    public:

      //! Constructs a RootItem.
      /*!
        \param name The name of the root item.
      */
      RootItem(const std::string& name);

      virtual ~RootItem();

      virtual Type GetType() const;
      virtual std::string GetName() const;
      virtual QIcon GetIcon() const;

    private:
      std::string m_name;
  };
}

#endif
