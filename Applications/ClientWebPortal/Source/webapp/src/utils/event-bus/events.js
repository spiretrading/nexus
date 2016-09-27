/** Web application internal hierarchical event name constants */
export default {
  All: 'all',

  // application wide events
  ALL_APPLICATION: 'all.application',
  Application: {
    SIGNED_IN: 'all.application.signedIn',
    PAGE_TRANSITIONED: 'all.application.pageTransitioned',
    SHOW_PRELOADER: 'all.application.showPreloader',
    HIDE_PRELOADER: 'all.application.hidePreloader',
    PRELOADER_HIDDEN: 'all.application.preloaderHidden',
    BLANK_PAGE_LOADED: 'all.application.blankPageLoaded',
    RENDERED: 'all.application.rendered'
  },
  TopNav: {
    CLOSE_SIDE_MENU: 'all.application.topNav.closeSideMenu',
    SIDE_MENU_OPENED: 'all.application.topNav.menuOpened',
    SIDE_MENU_CLOSED: 'all.application.topNav.menuClosed'
  }
};
