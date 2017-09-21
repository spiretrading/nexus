import {
  AdministrationClient,
  CurrencyId,
  DirectoryEntry,
  RiskServiceClient,
  MarketCode,
  Money
} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import PortfolioModel from './portfolio-model';
import SortModel from './sort-model';
import StringifyModel from './stringify-model';
import DataChangeType from './data-change-type';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.riskServiceClient = new RiskServiceClient();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();
    this.portfolioModel = new PortfolioModel();
    this.sortModel = new SortModel(this.portfolioModel);
    this.stringifyModel = new StringifyModel(this.sortModel);

    this.getRequiredData = this.getRequiredData.bind(this);
    this.setTableRef = this.setTableRef.bind(this);
    this.onDataModelChange = this.onDataModelChange.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  setTableRef(ref) {
    this.table = ref;
  }

  onDataModelChange(dataChangeType, rowIndex, toIndex) {
    if (dataChangeType == DataChangeType.ADD) {
      this.table.rowAdd(rowIndex);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.table.rowRemove(rowIndex);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.table.rowUpdate(rowIndex);
    } else if (dataChangeType == DataChangeType.MOVE) {
      this.table.rowMove(rowIndex, toIndex);
    }
  }

  /** @private */
  getRequiredData() {
    let loadManagedTradingGroups = this.adminClient.loadManagedTradingGroups(this.componentModel.directoryEntry);

    let directoryEntry = userService.getDirectoryEntry();
    let loadAccountRiskParameters = this.adminClient.loadRiskParameters(directoryEntry);

    return Promise.all([
      loadManagedTradingGroups,
      loadAccountRiskParameters
    ]);
  }

  /** @private */
  onPortfolioDataReceived(data) {
    this.portfolioModel.onDataReceived(data);
    this.componentModel.aggregates = this.portfolioModel.getTotals();
    this.view.update(this.componentModel);
  }

  /** @private */
  onFilterResize() {
    this.view.resizePortfolioChart();
  }

  componentDidMount() {
    this.dataModelChangeSubId = this.stringifyModel.addDataChangeListener(this.onDataModelChange);
    this.filterResizeSubId = EventBus.subscribe(Event.Portfolio.FILTER_RESIZE, this.onFilterResize.bind(this));

    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData();

    this.riskServiceClient.subscribePortfolio(this.onPortfolioDataReceived.bind(this))
      .then((subscriptionId) => {
        this.portfolioSubscriptionId = subscriptionId;
      });

    this.view.initialize();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.portfolioModel.setBaseCurrencyId(responses[1].currencyId);
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
          id: markets[i].marketCode.toData(),
          name: markets[i].displayName
        });
      }

      this.view.update(this.componentModel);
    });
  }

  componentWillUnmount() {
    this.stringifyModel.removeDataChangeListener(this.dataModelChangeSubId);
    this.riskServiceClient.unsubscribe(this.portfolioSubscriptionId);
    EventBus.unsubscribe(this.filterResizeSubId);
    this.view.dispose();
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  saveParameters(filter) {
    EventBus.publish(Event.Portfolio.FILTER_PARAMETERS_CHANGED);

    this.componentModel.filter = filter;

    let groups = [];
    for (let i=0; i<filter.groups.length; i++) {
      groups.push(DirectoryEntry.fromData(filter.groups[i]));
    }

    let currencies = [];
    for (let i=0; i<filter.currencies.length; i++) {
      currencies.push(CurrencyId.fromData(filter.currencies[i].id));
    }

    let marketCodes = [];
    for (let i=0; i<filter.markets.length; i++) {
      marketCodes.push(MarketCode.fromData(filter.markets[i].id));
    }

    let apiFilter = {
      currencies: currencies,
      groups: groups,
      markets: marketCodes
    };

    this.riskServiceClient.setPortfolioDataFilter(this.portfolioSubscriptionId, apiFilter);
    this.view.update(this.componentModel);
  }

  getDataModel() {
    return this.stringifyModel;
  }
}

export default Controller;
