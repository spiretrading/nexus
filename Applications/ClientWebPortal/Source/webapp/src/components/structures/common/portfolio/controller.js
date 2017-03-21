import {AdministrationClient, DirectoryEntry, RiskServiceClient} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import HashMap from 'hashmap';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.riskServiceClient = new RiskServiceClient();
    this.portfolioData = new HashMap();
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

  /** @private */
  onPortfolioMessageReceived(message) {
    let cacheKey = message.account + message.currency + message.security;
    this.portfolioData.set(cacheKey, message);
    this.componentModel.portfolioData = this.portfolioData.values();
    this.view.update(this.componentModel);
  }

  componentDidMount() {
    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData.apply(this);

    this.riskServiceClient.subscribePortfolio(this.onPortfolioMessageReceived.bind(this))
      .then((subscriptionId) => {
        this.portfolioSubscriptionId = subscriptionId;
      });

    this.view.initialize.apply(this.view);

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

  componentWillUnmount() {
    this.riskServiceClient.unsubscribe.apply(this.riskServiceClient, [this.portfolioSubscriptionId]);

    this.view.dispose.apply(this.view);
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  saveParameters(filter) {
    this.componentModel.filter = filter;
    let apiFilter = {
      currencies: filter.currencies,
      groups: filter.groups,
      markets: filter.markets
    };
    this.riskServiceClient.setFilter(apiFilter);
  }
}

export default Controller;
