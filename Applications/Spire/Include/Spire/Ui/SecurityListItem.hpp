#ifndef SPIRE_SECURITY_LIST_ITEM_HPP
#define SPIRE_SECURITY_LIST_ITEM_HPP
#include <QWidget>
#include "Nexus/Definitions/SecurityInfo.hpp"

namespace Spire {

  /** Represents a security list item in a SecurityBox. */
  class SecurityListItem : public QWidget {
    public:

      /**
       * Constructs a SecurityListItem.
       * @param security_info The security to be displayed in the
       *                      SecurityListItem.
       * @param parent The parent widget.
       */
      explicit SecurityListItem(
        Nexus::SecurityInfo security_info, QWidget* parent = nullptr);

      /** Returns the security. */
      const Nexus::SecurityInfo& get_security() const;

    private:
      Nexus::SecurityInfo m_security_info;
  };
}

#endif
