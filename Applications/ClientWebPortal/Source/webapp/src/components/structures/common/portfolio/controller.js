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
import DataChangeType from './data-change-type';
import tableColumns from './table-columns';
import HashMap from 'hashmap';
import ModelToCsvExporter from './model-to-csv-exporter';
import deviceDetector from 'utils/device-detector';
import PlainNumberViewModel from 'utils/table-models/plain-number-view-model';
import PortfolioModel from 'utils/table-models/portfolio-model';
import TypedViewModel from 'utils/table-models/typed-view-model';
import SubsetModel from 'utils/table-models/subset-model';
import SumModel from 'utils/table-models/sum-model';
import SortModel from 'utils/table-models/sort-model';
import TranslationModel from 'utils/table-models/translation-model';
import PositiveNegativeMoneyStyle from 'utils/table-models/style-rules/positive-negative-money';

const RENDER_THROTTLE_INTERVAL = 500;

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.riskServiceClient = new RiskServiceClient();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();
    this.columnSortOrders = [];

    this.getRequiredData = this.getRequiredData.bind(this);
    this.setTableRef = this.setTableRef.bind(this);
    this.onDataModelChange = this.onDataModelChange.bind(this);
    this.onTotalsSumModelDataChange = this.onTotalsSumModelDataChange.bind(this);
    this.changeSortOrder = this.changeSortOrder.bind(this);
    this.resizeTable = this.resizeTable.bind(this);
    this.onRenderThrottleCall = this.onRenderThrottleCall.bind(this);
    this.exportToCSV = this.exportToCSV.bind(this);
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

  componentDidMount() {
    this.filterResizeSubId = EventBus.subscribe(Event.Portfolio.FILTER_RESIZE, this.resizeTable);

    this.componentModel = {
      directoryEntry: userService.getDirectoryEntry()
    };
    let requiredDataFetchPromise = this.getRequiredData();
    this.view.initialize();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then(responses => {
      let baseCurrencyId = responses[1].currencyId;
      this.portfolioModel = new PortfolioModel(this.riskServiceClient, baseCurrencyId);
      this.viewModel = new TypedViewModel(this.portfolioModel, this.getColumnStyles(), 10);
      this.sortModel = new SortModel(this.viewModel, this.columnSortOrders);
      this.filterSubsetModel = new SubsetModel(this.sortModel, []);

      this.totalsSubsetModel = new SubsetModel(this.portfolioModel, [0, 1, 3, 13, 14, 15]);
      this.totalsTranslationModel = new TranslationModel(this.totalsSubsetModel, null, [7, 0, 1, 2, 3, 4, 5, 6, 8, 9]);
      this.totalsSumModel = new SumModel(this.totalsTranslationModel, baseCurrencyId);

      this.dataModelChangeSubId = this.filterSubsetModel.addDataChangeListener(this.onDataModelChange);
      this.dataSumChangeSubId = this.totalsSumModel.addDataChangeListener(this.onTotalsSumModelDataChange);

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

      this.componentModel.isInitialized = true;
      this.view.update(this.componentModel);
      this.renderThrottle = setInterval(this.onRenderThrottleCall, RENDER_THROTTLE_INTERVAL);
    });
  }

  componentWillUnmount() {
    this.filterSubsetModel.removeDataChangeListener(this.dataModelChangeSubId);
    this.totalsSumModel.removeDataChangeListener(this.dataSumChangeSubId);
    this.portfolioModel.dispose();
    EventBus.unsubscribe(this.filterResizeSubId);
    clearInterval(this.renderThrottle);
    this.view.dispose();
  }

  saveParameters(filter) {
    this.componentModel.filter = filter;

    // TODO: commented out until API filtering is implemented
    // let groups = [];
    // for (let i=0; i<filter.groups.length; i++) {
    //   groups.push(DirectoryEntry.fromData(filter.groups[i]));
    // }
    //
    // let currencies = [];
    // for (let i=0; i<filter.currencies.length; i++) {
    //   currencies.push(CurrencyId.fromData(filter.currencies[i].id));
    // }
    //
    // let marketCodes = [];
    // for (let i=0; i<filter.markets.length; i++) {
    //   marketCodes.push(MarketCode.fromData(filter.markets[i].id));
    // }
    //
    // let apiFilter = {
    //   currencies: currencies,
    //   groups: groups,
    //   markets: marketCodes
    // };
    //
    // this.riskServiceClient.setPortfolioDataFilter(this.portfolioSubscriptionId, apiFilter);

    this.updateColumnFilters(this.componentModel.filter.columns);
  }

  getDataModel() {
    return this.filterSubsetModel;
  }

  changeSortOrder(sortOrder) {
    this.columnSortOrders = sortOrder;
    this.sortModel.changeSortOrder(this.columnSortOrders);
  }

  exportToCSV() {
    let plainNumberViewModel = new PlainNumberViewModel(this.filterSubsetModel);
    let csvExporter = new ModelToCsvExporter(plainNumberViewModel);
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

  /** @private */
  onDataModelChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.table.rowAdd(payload);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.table.rowRemove(payload.index, payload.row);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.table.rowUpdate(payload.index, payload.original);
    } else if (dataChangeType == DataChangeType.MOVE) {
      this.table.rowMove(payload.from, payload.to);
    }
  }

  /** @private */
  onTotalsSumModelDataChange() {
    this.componentModel.aggregates = {
      totalPnL: this.totalsSumModel.getValueAt(3, 0),
      unrealizedPnL: this.totalsSumModel.getValueAt(4, 0),
      realizedPnL: this.totalsSumModel.getValueAt(5, 0),
      fees: this.totalsSumModel.getValueAt(6, 0),
      volumes: this.totalsSumModel.getValueAt(8, 0),
      trades: this.totalsSumModel.getValueAt(9, 0)
    };
  }

  /** @private */
  onRenderThrottleCall() {
    this.view.update(this.componentModel);
  }

  /** @private */
  updateColumnFilters(includedColumns) {
    // clean up models
    this.filterSubsetModel.removeDataChangeListener(this.dataModelChangeSubId);
    this.filterSubsetModel.dispose();

    // construct and chain new models
    let omittedColumns = this.getOmittedColumns(includedColumns);
    this.removeFromSortOrders(omittedColumns);
    this.sortModel.changeSortOrder(this.columnSortOrders);
    this.filterSubsetModel = new SubsetModel(this.sortModel, omittedColumns);
    this.dataModelChangeSubId = this.filterSubsetModel.addDataChangeListener(this.onDataModelChange);

    // update big table of the changes
    this.table.updateColumnChange(this.filterSubsetModel, omittedColumns);
  }

  /** @private */
  removeFromSortOrders(omittedColumns) {
    this.columnSortOrders = this.columnSortOrders.filter(element => {
      return !omittedColumns.includes(element.index);
    });
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

  /** @private */
  getColumnStyles() {
    let posNegMoney = new PositiveNegativeMoneyStyle();
    return {
      'Total P&L': posNegMoney,
      'Unrealized P&L': posNegMoney,
      'Realized P&L': posNegMoney,
      'Acc. Total P&L': posNegMoney,
      'Acc. Unrealized P&L': posNegMoney
    };
  }
}

export default Controller;
