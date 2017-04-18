import Routes from './routes';

class MobileRoutes extends Routes {
  getRoutes() {
    let loadRoute = this.loadRoute.bind(this);
    let errorLoading = this.errorLoading.bind(this);
    return [
      {
        path: '/',
        getComponent(location, cb) {
          System.import('../components/structures/common/home/index')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-account/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.groupAccount } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-compliance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.compliance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-performance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.performance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-account/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.account } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'blank',
        getComponent(location, cb) {
          System.import('../components/structures/common/blank/index')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-riskControls/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.riskControls } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-entitlements/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.entitlements } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-compliance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/profile/routes')
            .then((module) => { return { default: module.compliance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-performance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.performance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'searchProfiles-search',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/search-profiles/routes')
            .then((module) => { return { default: module.searchProfiles } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'searchProfiles-newAccount',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/search-profiles/routes')
            .then((module) => { return { default: module.newAccount } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'portfolio',
        getComponent(location, cb) {
          System.import('../components/structures/mobile/portfolio/index')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      }
    ];
  }
}

export default new MobileRoutes();
