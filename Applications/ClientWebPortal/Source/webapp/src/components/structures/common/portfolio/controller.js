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
import ColumnSubsetModel from './column-subset-model';
import ColumnSumModel from './column-sum-model';
import ViewModel from './portfolio-view-model';
import DataChangeType from './data-change-type';
import tableColumns from './table-columns';
import HashMap from 'hashmap';
import ValueComparer from './value-comparer';
import ModelToCsvExporter from './model-to-csv-exporter';
import deviceDetector from 'utils/device-detector';
import PlainNumberModel from './plain-number-model';

const RENDER_THROTTLE_INTERVAL = 500;

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.riskServiceClient = new RiskServiceClient();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();
    this.portfolioModel = new PortfolioModel();
    this.sortModel = new SortModel(this.portfolioModel, [
      { index: 0, isAsc: true },
      { index: 1, isAsc: true },
      { index: 2, isAsc: true },
      { index: 3, isAsc: true },
      { index: 4, isAsc: true },
      { index: 5, isAsc: true },
      { index: 6, isAsc: true },
      { index: 7, isAsc: true },
      { index: 8, isAsc: true },
      { index: 9, isAsc: true },
      { index: 10, isAsc: true },
      { index: 11, isAsc: true },
      { index: 12, isAsc: true },
      { index: 13, isAsc: true },
      { index: 14, isAsc: true },
      { index: 15, isAsc: true }
    ], ValueComparer);
    this.viewModel = new ViewModel(this.sortModel);
    this.filterSubsetModel = new ColumnSubsetModel(this.viewModel, []);

    this.totalSubsetModel = new ColumnSubsetModel(this.portfolioModel, [0, 1, 3, 10]);
    this.sumModel = new ColumnSumModel(this.totalSubsetModel);

    this.getRequiredData = this.getRequiredData.bind(this);
    this.setTableRef = this.setTableRef.bind(this);
    this.onDataModelChange = this.onDataModelChange.bind(this);
    this.onSumModelDataChange = this.onSumModelDataChange.bind(this);
    this.changeSortOrder = this.changeSortOrder.bind(this);
    this.resizeTable = this.resizeTable.bind(this);
    this.onRenderThrottleCall = this.onRenderThrottleCall.bind(this);
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
  }

  /** @private */
  resizeTable() {
    this.table.resize();
  }

  onSumModelDataChange() {
    this.componentModel.aggregates = {
      totalPnL: this.sumModel.getValueAt(2, 0),
      unrealizedPnL: this.sumModel.getValueAt(3, 0),
      realizedPnL: this.sumModel.getValueAt(4, 0),
      fees: this.sumModel.getValueAt(5, 0),
      volumes: this.sumModel.getValueAt(7, 0),
      trades: this.sumModel.getValueAt(8, 0)
    };
  }

  /** @private */
  onRenderThrottleCall() {
    this.view.update(this.componentModel);
  }

  componentDidMount() {
    this.dataModelChangeSubId = this.filterSubsetModel.addDataChangeListener(this.onDataModelChange);
    this.dataSumChangeSubId = this.sumModel.addDataChangeListener(this.onSumModelDataChange);
    this.filterResizeSubId = EventBus.subscribe(Event.Portfolio.FILTER_RESIZE, this.resizeTable);

    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData();

    this.riskServiceClient.subscribePortfolio(this.onPortfolioDataReceived.bind(this))
      .then((subscriptionId) => {
        this.portfolioSubscriptionId = subscriptionId;
      });

    this.view.initialize();

    this.renderThrottle = setInterval(this.onRenderThrottleCall, RENDER_THROTTLE_INTERVAL);

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
    this.filterSubsetModel.removeDataChangeListener(this.dataModelChangeSubId);
    this.sumModel.removeDataChangeListener(this.dataSumChangeSubId);
    this.riskServiceClient.unsubscribePortfolio(this.portfolioSubscriptionId);
    EventBus.unsubscribe(this.filterResizeSubId);
    clearInterval(this.renderThrottle);
    this.view.dispose();
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  saveParameters(filter) {
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
    this.updateColumnFilters(this.componentModel.filter.columns);
  }

  /** @private */
  updateColumnFilters(includedColumns) {
    // clean up models
    this.filterSubsetModel.removeDataChangeListener(this.dataModelChangeSubId);
    this.filterSubsetModel.dispose();

    // construct and chain new models
    let omittedColumns = this.getOmittedColumns(includedColumns);
    this.filterSubsetModel = new ColumnSubsetModel(this.viewModel, omittedColumns);
    this.dataModelChangeSubId = this.filterSubsetModel.addDataChangeListener(this.onDataModelChange);

    // update big table of the changes
    this.table.updateColumnChange(this.filterSubsetModel);
  }

  /** @private */
  getOmittedColumns(includedColumns) {
    // map of column ids of included columns
    let includedColumnsMap = new HashMap();
    for (let i=0; i<includedColumns.length; i++) {
      includedColumnsMap.set(includedColumns[i].id, true);
    }

    let omittedColumns = [];
    for (let i=0; i<tableColumns.length; i++) {
      if (!tableColumns[i].isPrimaryKey && !includedColumnsMap.has(tableColumns[i].id)) {
        omittedColumns.push(i);
      }
    }

    return omittedColumns;
  }

  getDataModel() {
    return this.filterSubsetModel;
  }

  changeSortOrder(sortOrder) {
    this.sortModel.changeSortOrder(sortOrder);
  }

  exportToCSV() {
    let plainNumberModel = new PlainNumberModel(this.filterSubsetModel);
    let csvExporter = new ModelToCsvExporter(plainNumberModel);
    let csvString = csvExporter.getCsvString();

    if (deviceDetector.isInternetExplorer()) {
      var blob = new Blob([decodeURIComponent(encodeURI(csvString))], {
        type: "text/csv;charset=utf-8;"
      });
      navigator.msSaveBlob(blob, 'portfolio.csv');
    } else {
      let a = document.createElement('a');
      let csvData = new Blob([csvString], { type: 'text/csv' });
      let csvUrl = URL.createObjectURL(csvData);
      a.href = csvUrl;
      a.target = '_blank';
      a.download = 'portfolio.csv';
      a.click();
    }
  }
}

export default Controller;
