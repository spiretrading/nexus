#ifndef SPIRE_REGION_INPUT_WIDGET_HPP
#define SPIRE_REGION_INPUT_WIDGET_HPP
#include <Beam/Pointers/Ref.hpp>
#include <boost/signals2/signal.hpp>
#include <QWidget>
#include "Nexus/Definitions/Region.hpp"
#include "Spire/LegacyUI/UserProfile.hpp"

class QLineEdit;

namespace Spire {

  /** Displays a widget to input and edit a Region. */
  class RegionInputWidget : public QWidget {
    public:

      /**
       * Signals a change to the value.
       * @param region The updated Region.
       */
      using UpdatedSignal =
        boost::signals2::signal<void (const Nexus::Region& region)>;

      /**
       * Constructs a RegionInputWidget.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      explicit RegionInputWidget(
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /**
       * Constructs a RegionInputWidget.
       * @param region The Region to display.
       * @param userProfile The user's profile.
       * @param parent The parent widget.
       */
      RegionInputWidget(Nexus::Region region,
        Beam::Ref<UserProfile> userProfile, QWidget* parent = nullptr);

      /** Returns the Region. */
      const Nexus::Region& GetRegion() const;

      /** Sets the Region. */
      void SetRegion(const Nexus::Region& region);

      /**
       * Makes this widget read-only.
       * @param value <code>true</code> to make this widget read-only.
       */
      void SetReadOnly(bool value);

      /**
       * Connects a slot to the UpdatedSignal.
       * @param slot The slot to connect.
       * @return A connection to the signal.
       */
      boost::signals2::connection ConnectUpdatedSignal(
        const UpdatedSignal::slot_type& slot) const;

    protected:
      bool eventFilter(QObject* receiver, QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void mouseDoubleClickEvent(QMouseEvent* event) override;

    private:
      UserProfile* m_userProfile;
      Nexus::Region m_region;
      bool m_isReadOnly;
      QLineEdit* m_lineEdit;
      mutable UpdatedSignal m_updatedSignal;
  };
}

#endif
