import Routes from './routes';

class DesktopRoutes extends Routes {
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
        path: 'blank',
        getComponent(location, cb) {
          System.import('../components/structures/common/blank/index')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-account/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.groupAccount } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-compliance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.compliance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'group-profile-performance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.performance } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-account/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.account } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-riskControls/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.riskControls } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-entitlements/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
            .then((module) => { return { default: module.entitlements } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'profile-compliance/:type/:id/:name',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/profile/routes')
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
        path: 'accounts',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/accounts/routes')
            .then((module) => { return { default: module.accounts } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'accounts-newAccount',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/accounts/routes')
            .then((module) => { return { default: module.newAccount } })
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'portfolio',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/portfolio/index')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'modification-request-history',
        getComponent(location, cb) {
          System.import('../components/structures/desktop/modification-request-history')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      },
      {
        path: 'entitlement-modification-review',
        getComponent(location, cb) {
          System.import('../components/structures/common/entitlement-modification-review')
            .then(loadRoute(cb))
            .catch(errorLoading);
        }
      }
    ];
  }
}

export default new DesktopRoutes();
