import {AdministrationClient, DirectoryEntry} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
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
    let loadManagedTradingGroups = this.adminClient.loadManagedTradingGroups.apply(
      this.adminClient,
      [(this.componentModel.directoryEntry)]
    );

    return Promise.all([
      loadManagedTradingGroups
    ]);
  }

  componentDidMount() {
    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData.apply(this);

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.isAdmin = userService.isAdmin();

      // groups
      this.componentModel.managedGroups = responses[0];
      this.componentModel.managedGroups.sort((a,b) => {
        if (a.name.toLowerCase() < b.name.toLowerCase()) return -1;
        if (a.name.toLowerCase() > b.name.toLowerCase()) return 1;
        return 0;
      });

      // currencies
      let currencies = definitionsService.getAllCurrencies();
      this.componentModel.currencies = [];
      for (let i=0; i<currencies.length; i++) {
        this.componentModel.currencies.push({
          id: currencies[i].id.value,
          name: currencies[i].code
        });
      }

      // markets
      let markets = definitionsService.getAllMarkets();
      this.componentModel.markets = [];
      for (let i=0; i<markets.length; i++) {
        this.componentModel.markets.push({
          id: markets[i].code,
          name: markets[i].display_name
        });
      }

      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  saveParameters() {

  }
}

export default Controller;
