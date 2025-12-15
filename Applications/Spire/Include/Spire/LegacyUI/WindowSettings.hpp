#ifndef SPIRE_LEGACY_UI_WINDOW_SETTINGS_HPP
#define SPIRE_LEGACY_UI_WINDOW_SETTINGS_HPP
#include <memory>
#include <string>
#include <vector>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <QByteArray>
#include "Spire/Spire/Spire.hpp"

class QWidget;

namespace Spire {
  class UserProfile;

namespace LegacyUI {

  /** Stores the information needed to re-open a window. */
  class WindowSettings {
    public:

      /**
       * Loads a profile's window settings.
       * @param userProfile The user's profile.
       */
      static std::vector<std::unique_ptr<WindowSettings>>
        Load(const UserProfile& userProfile);

      /**
       * Saves a profile's window settings.
       * @param userProfile The user's profile.
       */
      static void Save(const UserProfile& userProfile);

      virtual ~WindowSettings() = default;

      /** Returns the name of the window. */
      virtual std::string GetName() const;

      /**
       * Re-opens the window.
       * @param userProfile The user's profile.
       */
      virtual QWidget* Reopen(Beam::Ref<UserProfile> userProfile) const = 0;

      /**
       * Applies the settings to an existing window/widget.
       * @param userProfile The user's profile.
       * @param widget The widget to apply the settings to.
       */
      virtual void Apply(Beam::Ref<UserProfile> userProfile,
        Beam::Out<QWidget> widget) const = 0;
  };

  /**
   * Restores a QWidget's geometry stored in a QByteArray. This mostly
   * reimplements Qt's QWidget::restoreGeometry() but also handles changes in
   * DPI and resolution.
   * @param widget The widget whose geometry is to be restored.
   * @param geometry The byte array containing the geometry.
   */
  void restore_geometry(QWidget& widget, const QByteArray& geometry);
}
}

#endif
