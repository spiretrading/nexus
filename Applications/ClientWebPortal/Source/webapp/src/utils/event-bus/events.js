/** Web application internal hierarchical event name constants */
export default {
  All: 'all',

  // application wide events
  ALL_APPLICATION: 'all.application',
  Application: {
    SIGNED_IN: 'all.application.signedIn',
    PAGE_TRANSITIONED: 'all.application.pageTransitioned',
    PAGE_LOADING: 'all.application.pageLoading',
    PAGE_LOADED: 'all.application.pageLoaded',
    TRANSITION_PAGE: 'all.application.transitionPage',
    SHOW_PRELOADER: 'all.application.showPreloader',
    HIDE_PRELOADER: 'all.application.hidePreloader',
    PRELOADER_HIDDEN: 'all.application.preloaderHidden'
  },
  TopNav: {
    MENU_OPENED: 'all.application.topNave.menuOpened'
  }
};