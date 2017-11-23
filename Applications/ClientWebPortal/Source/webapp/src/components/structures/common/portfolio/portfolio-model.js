import {
  Money
} from 'spire-client';
import HashMap from 'hashmap';
import definitionsService from 'services/definitions';
import DataChangeType from './data-change-type';
import ChainableModel from './chainable-model';
import tableColumns from './table-columns';

class PortfolioModel extends ChainableModel{
  constructor() {
    super(null);
    this.dataIndices = new HashMap();
    this.data = [];
    this.accountTotals = new HashMap();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();
  }

  onDataReceived(data) {
    for (let i=0; i<data.length; i++) {
      let parsedData = this.parse(data[i]);
      let cacheKey = parsedData.account.id + parsedData.currency.value + parsedData.security.market.value + parsedData.security.symbol;

      if (parsedData.trades == 0 && this.dataIndices.has(cacheKey)) {
        // remove any row for which the streaming has stopped
        let rowIndex = this.dataIndices.get(cacheKey);
        let data = this.data[rowIndex];
        this.removeFromAggregates(data);
        this.data.splice(rowIndex, 1);
        this.dataIndices.remove(cacheKey);
        this.signalDataChanged(DataChangeType.REMOVE, rowIndex);
      } else if (!this.dataIndices.has(cacheKey)) {
        // new data
        this.data.push(parsedData);
        let rowIndex = this.data.length - 1;
        this.dataIndices.set(cacheKey, this.data.length - 1);
        if (this.baseCurrencyId != null) {
          this.aggregateNewDataTotals(parsedData);
        }
        this.signalDataChanged(DataChangeType.ADD, rowIndex);
      } else {
        // existing data
        let rowIndex = this.dataIndices.get(cacheKey);
        let oldData = clone(this.data[rowIndex]);
        this.data[rowIndex] = parsedData;
        if (this.baseCurrencyId != null) {
          this.aggregateExistingDataTotals(oldData, parsedData);
        }
        this.signalDataChanged(DataChangeType.UPDATE, rowIndex);
      }
    }
  }

  setBaseCurrencyId(currencyId) {
    this.baseCurrencyId = currencyId;
  }

  getValueAt(x, y) {
    let row = this.data[y];
    switch(x) {
      case 0:
        return row.account;
      case 1:
        return row.security;
      case 2:
        return row.quantity;
      case 3:
        return row.side;
      case 4:
        return row.averagePrice;
      case 5:
        return row.totalPnL;
      case 6:
        return row.unrealizedPnL;
      case 7:
        return row.realizedPnL;
      case 8:
        return row.fees;
      case 9:
        return row.costBasis;
      case 10:
        return row.currency;
      case 11:
        return row.volume;
      case 12:
        return row.trades;
      case 13:
        return this.accountTotals.get(row.account.id).totalPnL;
      case 14:
        return this.accountTotals.get(row.account.id).unrealizedPnL;
      case 15:
        return this.accountTotals.get(row.account.id).fees;
    }
  }

  getColumnHeader(x) {
    return tableColumns[x].name;
  }

  getCurrencyForRow(y) {
    return this.data[y].currency;
  }

  getRowCount() {
    return this.data.length;
  }

  getColumnCount() {
    return 16;
  }

  /** @private */
  parse(data) {
    let row = {};

    row.account = data.account;
    row.security = data.inventory.position.key.index;
    row.quantity = data.inventory.position.quantity;

    if (data.inventory.position.quantity > 0) {
      row.side = 'Long';
    } else if (data.inventory.position.quantity < 0) {
      row.side = 'Short';
    } else {
      row.side = 'Flat';
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
    row.averagePrice = averagePrice;

    let totalPnL = null;
    let unrealizedPnL = null;
    let grossPnL = data.inventory.gross_profit_and_loss;
    let fees = data.inventory.fees;
    if (data.unrealized_profit_and_loss.is_initialized) {
      unrealizedPnL = data.unrealized_profit_and_loss.value;
      totalPnL = unrealizedPnL.add(grossPnL).subtract(fees);
    }
    row.totalPnL = totalPnL;
    row.unrealizedPnL = unrealizedPnL;
    row.realizedPnL = grossPnL.subtract(fees);
    row.fees = fees;
    row.costBasis = data.inventory.position.cost_basis;
    row.currency = data.inventory.position.key.currency;
    row.volume = data.inventory.volume;
    row.trades = data.inventory.transaction_count;

    return row;
  }

  /** @private */
  removeFromAggregates(data) {
    // account total
    let accountTotal = this.accountTotals.get(data.account.id);
    accountTotal.totalPnL.subtract(data.totalPnL);
    accountTotal.unrealizedPnL.subtract(data.unrealizedPnL);
    accountTotal.fees.subtract(data.fees);
  }

  /** @private */
  aggregateExistingDataTotals(oldData, newData) {
    let accountTotal;
    if (!this.accountTotals.has(newData.account.id)) {
      accountTotal = {
        totalPnL: Money.fromNumber(0),
        unrealizedPnL: Money.fromNumber(0),
        fees: Money.fromNumber(0)
      };
      this.accountTotals.set(newData.account.id, accountTotal);
    } else {
      accountTotal = this.accountTotals.get(newData.account.id);
    }

    // totalPnL
    let oldTotalPnL = oldData.totalPnL || Money.fromNumber(0);
    let newTotalPnL = newData.totalPnL || Money.fromNumber(0);
    let totalPnLDifference = newTotalPnL.subtract(oldTotalPnL);
    let convertedTotalPnL = this.convertCurrencies(
      newData.currency,
      this.baseCurrencyId,
      totalPnLDifference
    );
    accountTotal.totalPnL = accountTotal.totalPnL.add(convertedTotalPnL);

    // unrealizedPnL
    let oldUnrealizedPnL = oldData.unrealizedPnL || Money.fromNumber(0);
    let newUnrealizedPnL = newData.unrealizedPnL || Money.fromNumber(0);
    let unrealizedPnLDifference = newUnrealizedPnL.subtract(oldUnrealizedPnL);
    let convertedUnrealizedPnL = this.convertCurrencies(
      newData.currency,
      this.baseCurrencyId,
      unrealizedPnLDifference
    );
    accountTotal.unrealizedPnL = accountTotal.unrealizedPnL.add(convertedUnrealizedPnL);

    // realizedPnL
    let oldRealizedPnL = oldData.realizedPnL || Money.fromNumber(0);
    let newRealizedPnL = newData.realizedPnL || Money.fromNumber(0);
    let realizedPnLDifference = newRealizedPnL.subtract(oldRealizedPnL);
    let convertedRealizedPnL = this.convertCurrencies(
      newData.currency,
      this.baseCurrencyId,
      realizedPnLDifference
    );

    // fees
    let oldFees = oldData.fees || Money.fromNumber(0);
    let newFees = newData.fees || Money.fromNumber(0);
    let feesDifference = newFees.subtract(oldFees);
    let convertedFees = this.convertCurrencies(
      newData.currency,
      this.baseCurrencyId,
      feesDifference
    );
    accountTotal.fees = accountTotal.fees.add(convertedFees);
  }

  /** @private */
  aggregateNewDataTotals(data) {
    let accountTotal;
    if (!this.accountTotals.has(data.account.id)) {
      accountTotal = {
        totalPnL: Money.fromNumber(0),
        unrealizedPnL: Money.fromNumber(0),
        fees: Money.fromNumber(0)
      };
      this.accountTotals.set(data.account.id, accountTotal);
    } else {
      accountTotal = this.accountTotals.get(data.account.id);
    }

    // totalPnL
    let originalTotalPnL = data.totalPnL || Money.fromNumber(0);
    let convertedTotalPnL = this.convertCurrencies(
      data.currency,
      this.baseCurrencyId,
      originalTotalPnL
    );
    accountTotal.totalPnL = accountTotal.totalPnL.add(convertedTotalPnL);

    // unrealizedPnL
    let originalUnrealizedPnL = data.unrealizedPnL || Money.fromNumber(0);
    let convertedUnrealizedPnL = this.convertCurrencies(
      data.currency,
      this.baseCurrencyId,
      originalUnrealizedPnL
    );
    accountTotal.unrealizedPnL = accountTotal.unrealizedPnL.add(convertedUnrealizedPnL);

    // realizedPnL
    let originalRealizedPnL = data.realizedPnL || Money.fromNumber(0);
    let convertedRealizedPnL = this.convertCurrencies(
      data.currency,
      this.baseCurrencyId,
      originalRealizedPnL
    );

    // fees
    let originalFees = data.fees || Money.fromNumber(0);
    let convertedFees = this.convertCurrencies(
      data.currency,
      this.baseCurrencyId,
      originalFees
    );
    accountTotal.fees = accountTotal.fees.add(convertedFees);
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
  signalDataChanged(dataChangeType, rowIndex) {
    let listeners = this.dataChangeListeners.values();
    for (let i=0; i<listeners.length; i++) {
      listeners[i](dataChangeType, rowIndex);
    }
  }
}

export default PortfolioModel;
