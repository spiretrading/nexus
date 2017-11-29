import {AdministrationClient, DirectoryEntry} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRoles = this.adminClient.loadAccountRoles(directoryEntry);

    return Promise.all([
      loadAccountRoles
    ]);
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.isGroup = directoryEntry.type === 1;
      this.componentModel.roles = responses[0];
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);

      EventBus.publish(Event.Profile.VIEWING_CONTEXT_LOADED, {
        directoryEntry: this.componentModel.directoryEntry,
        roles: this.componentModel.roles
      });
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  generate(startDate, endDate) {
    let mockDataPromise = new Promise((resolve, reject) => {
      setTimeout(() => {
        resolve({
          'AUD': [
            {
              symbol: 'ABC',
              pnl: 24.52,
              volume: 1000
            },
            {
              symbol: 'KDJ',
              pnl: 35.12,
              volume: 500
            },
            {
              symbol: 'IQE',
              pnl: 12123.53,
              volume: 28123
            }
          ],
          'CAD': [
            {
              symbol: 'ABC',
              pnl: 24.52,
              volume: 1000
            },
            {
              symbol: 'KDJ',
              pnl: 35.12,
              volume: 500
            },
            {
              symbol: 'IQE',
              pnl: 12123.53,
              volume: 28123
            }
          ],
          'USD': [
            {
              symbol: 'ABC',
              pnl: 24.52,
              volume: 1000
            },
            {
              symbol: 'KDJ',
              pnl: 35.12,
              volume: 500
            },
            {
              symbol: 'IQE',
              pnl: 12123.53,
              volume: 28123
            }
          ]
        });
      }, 5000);
    });

    preloaderTimer.start(
      mockDataPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((pnlData) => {
      this.componentModel.startDate = startDate;
      this.componentModel.endDate = endDate;
      this.componentModel.pnlReport = pnlData;
      this.view.update(this.componentModel);
    });
  }
}

export default Controller;
