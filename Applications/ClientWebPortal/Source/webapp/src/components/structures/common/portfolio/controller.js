import {
  AdministrationClient,
  DirectoryEntry,
  RiskServiceClient,
  Money
} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';
import definitionsService from 'services/definitions';
import HashMap from 'hashmap';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.adminClient = new AdministrationClient();
    this.riskServiceClient = new RiskServiceClient();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();
    this.portfolioData = new HashMap();

    this.toUIModel = this.toUIModel.bind(this);
    this.aggregateTotals = this.aggregateTotals.bind(this);
    this.convertCurrencies = this.convertCurrencies.bind(this);
    this.getRequiredData = this.getRequiredData.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
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
  toUIModel(data) {
    let model = {};

    model.account = data.account;
    model.security = data.inventory.position.key.index;
    model.quantity = data.inventory.position.quantity;

    if (data.inventory.position.quantity > 0) {
      model.side = 'Long';
    } else if (data.inventory.position.quantity < 0) {
      model.side = 'Short';
    } else {
      model.side = 'Flat';
    }

    let averagePrice = null;
    if (data.inventory.position.quantity != 0) {
      let costBasis = data.inventory.position.cost_basis;
      let quantity = data.inventory.position.quantity;
      averagePrice = costBasis.divide(quantity);
      if (averagePrice.toNumber() < 0) {
        averagePrice.multiply(-1);
      }
    }
    model.averagePrice = averagePrice;

    let totalPnL = null;
    let unrealizedPnL = null;
    let grossPnL = data.inventory.gross_profit_and_loss;
    let fees = data.inventory.fees;
    if (data.unrealized_profit_and_loss.is_initialized) {
      unrealizedPnL = data.unrealized_profit_and_loss.value;
      totalPnL = unrealizedPnL.add(grossPnL).subtract(fees);
    }
    model.totalPnL = totalPnL;
    model.unrealizedPnL = unrealizedPnL;
    model.realizedPnL = grossPnL.subtract(fees);
    model.fees = fees;
    model.costBasis = data.inventory.position.cost_basis;
    model.currency = data.inventory.position.key.currency;
    model.volume = data.inventory.volume;
    model.trades = data.inventory.transaction_count;

    return model;
  }

  /** @private */
  onPortfolioDataReceived(data) {
    for (let i=0; i<data.length; i++) {
      data[i] = this.toUIModel(data[i]);
      let cacheKey = data[i].account.id + data[i].currency.value + data[i].security.market.value + data[i].security.symbol;
      this.portfolioData.set(cacheKey, data[i]);
    }
    this.componentModel.portfolioData = this.portfolioData.values();

    if (this.componentModel.baseCurrencyId != null) {
      this.aggregateTotals();
    }
    this.view.update(this.componentModel);
  }

  /** @private */
  aggregateTotals() {
    let portfolioData = this.componentModel.portfolioData;
    let totalPnL = Money.fromNumber(0);
    let unrealizedPnL = Money.fromNumber(0);
    let realizedPnL = Money.fromNumber(0);
    let fees = Money.fromNumber(0);
    let volumes = 0;
    let trades = 0;
    let accountTotals = new HashMap();
    for (let i=0; i<portfolioData.length; i++) {
      let accountTotal;
      if (!accountTotals.has(portfolioData[i].account.id)) {
        accountTotal = {
          totalPnL: Money.fromNumber(0),
          unrealizedPnL: Money.fromNumber(0),
          fees: Money.fromNumber(0)
        };
        accountTotals.set(portfolioData[i].account.id, accountTotal);
      } else {
        accountTotal = accountTotals.get(portfolioData[i].account.id);
      }

      let originalTotalPnL = portfolioData[i].totalPnL || Money.fromNumber(0);
      let convertedTotalPnL = this.convertCurrencies(
        portfolioData[i].currency,
        this.componentModel.baseCurrencyId,
        originalTotalPnL
      );
      totalPnL = totalPnL.add(convertedTotalPnL);
      accountTotal.totalPnL = accountTotal.totalPnL.add(convertedTotalPnL);

      let originalUnrealizedPnL = portfolioData[i].unrealizedPnL || Money.fromNumber(0);
      let convertedUnrealizedPnL = this.convertCurrencies(
        portfolioData[i].currency,
        this.componentModel.baseCurrencyId,
        originalUnrealizedPnL
      );
      unrealizedPnL = unrealizedPnL.add(convertedUnrealizedPnL);
      accountTotal.unrealizedPnL = accountTotal.unrealizedPnL.add(convertedUnrealizedPnL);

      let originalRealizedPnL = portfolioData[i].realizedPnL || Money.fromNumber(0);
      let convertedRealizedPnL = this.convertCurrencies(
        portfolioData[i].currency,
        this.componentModel.baseCurrencyId,
        originalRealizedPnL
      );
      realizedPnL = realizedPnL.add(convertedRealizedPnL);

      let originalFees = portfolioData[i].fees || Money.fromNumber(0);
      let convertedFees = this.convertCurrencies(
        portfolioData[i].currency,
        this.componentModel.baseCurrencyId,
        originalFees
      );
      fees = fees.add(convertedFees);
      accountTotal.fees = accountTotal.fees.add(convertedFees);

      let volume = portfolioData[i].volume || 0;
      volumes += volume

      let trade = portfolioData[i].trades || 0;
      trades += trade;
    }

    for (let i=0; i<portfolioData.length; i++) {
      let accountId = portfolioData[i].account.id;
      let accountTotal = accountTotals.get(accountId);
      portfolioData[i].accountTotalPnL = accountTotal.totalPnL;
      portfolioData[i].accountUnrealizedPnL = accountTotal.unrealizedPnL;
      portfolioData[i].accountFees = accountTotal.fees;
    }

    this.componentModel.aggregates = {
      totalPnL: totalPnL,
      unrealizedPnL: unrealizedPnL,
      realizedPnL: realizedPnL,
      fees: fees,
      volumes: volumes,
      trades: trades
    };
  }

  /** @private */
  convertCurrencies(fromCurrencyId, toCurrencyId, amount) {
    if (fromCurrencyId.toNumber() == toCurrencyId.toNumber()) {
      return Money.fromNumber(amount);
    } else {
      return this.exchangeRateTable.convert(
        amount,
        fromCurrencyId,
        toCurrencyId
      );
    }
  }

  /** @private */
  onFilterResize() {
    this.view.resizePortfolioChart();
  }

  componentDidMount() {
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
      this.componentModel.baseCurrencyId = responses[1].currencyId;
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
          id: markets[i].marketCode.toCode(),
          name: markets[i].displayName
        });
      }

      this.view.update(this.componentModel);
    });
  }

  componentWillUnmount() {
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
    let apiFilter = {
      currencies: filter.currencies,
      groups: filter.groups,
      markets: filter.markets
    };
    this.riskServiceClient.setFilter(apiFilter);
    this.view.update(this.componentModel);
  }
}

export default Controller;
