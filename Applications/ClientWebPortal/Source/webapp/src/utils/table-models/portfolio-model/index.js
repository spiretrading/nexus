import Model from 'utils/table-models/model';
import IndexedModel from 'utils/table-models/indexed-model';
import HashMap from 'hashmap';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';
import definitionsService from 'services/definitions';
import { Money } from 'spire-client';

const KEY_INDICES = [0, 1, 10];
const COLUMN_NAMES = [
  'Account',
  'Security',
  'Quantity',
  'Side',
  'Average Price',
  'Total P&L',
  'Unrealized P&L',
  'Realized P&L',
  'Fees',
  'Cost Basis',
  'Currency',
  'Volume',
  'Trades'
];

const ACCOUNT_TOTAL_COLUMN_NAMES = [
  'Acc. Total P&L',
  'Acc. Unrealized P&L',
  'Acc. Fees'
];

export default class extends Model {
  constructor(riskServiceClient, baseCurrencyId) {
    super();
    this.riskServiceClient = riskServiceClient;
    this.baseCurrencyId = baseCurrencyId;
    this.signalManager = new SignalManager();
    this.indexedModel = new IndexedModel(KEY_INDICES, COLUMN_NAMES);
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.indexedModel.addDataChangeListener(this.onDataChange);
    this.onDataReceived = this.onDataReceived.bind(this);
    this.accountTotals = new HashMap();
    this.exchangeRateTable = definitionsService.getExchangeRateTable();

    this.riskServiceClient.subscribePortfolio(this.onDataReceived)
      .then(subscriptionId => {
        this.subscriptionId = subscriptionId;
      });
  }

  getRowCount() {
    return this.indexedModel.getRowCount();
  }

  getColumnCount() {
    return this.indexedModel.getColumnCount() + ACCOUNT_TOTAL_COLUMN_NAMES.length;
  }

  getValueAt(x, y) {
    if (x < 13) {
      return this.indexedModel.getValueAt(x, y);
    } else {
      let directoryEntry = this.indexedModel.getValueAt(0, y);
      let accountTotals = this.accountTotals.get(directoryEntry.id);
      if (x == 13) {
        return accountTotals.totalPnL;
      } else if (x == 14) {
        return accountTotals.unrealizedPnL;
      } else if (x == 15) {
        return accountTotals.fees;
      }
    }
  }

  getColumnName(columnIndex) {
    if (columnIndex < 13) {
      return this.indexedModel.getColumnName(columnIndex);
    } else {
      return ACCOUNT_TOTAL_COLUMN_NAMES[columnIndex - 13];
    }
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }

  dispose() {
    this.indexedModel.removeDataChangeListener(this.dataChangeSubId);
    this.riskServiceClient.unsubscribePortfolio(this.subscriptionId);
  }

  /** @private */
  onDataReceived(data) {
    for (let i=0; i<data.length; i++) {
      let rowData = this.toRowData(data[i]);
      if (data[i].inventory.transaction_count == 0) {
        this.indexedModel.removeRow(rowData);
      } else {
        this.indexedModel.update(rowData);
      }
    }
  }

  /** @private */
  toRowData(data) {
    let rowData = [];
    // account
    rowData.push(data.account);
    // security
    rowData.push(data.inventory.position.key.index);
    // quantity
    rowData.push(data.inventory.position.quantity);
    // side
    if (data.inventory.position.quantity > 0) {
      rowData.push('Long');
    } else if (data.inventory.position.quantity < 0) {
      rowData.push('Short');
    } else {
      rowData.push('Flat');
    }
    // average price
    let averagePrice = null;
    if (data.inventory.position.quantity != 0) {
      let costBasis = data.inventory.position.cost_basis;
      let quantity = data.inventory.position.quantity;
      averagePrice = costBasis.divide(quantity);
      if (averagePrice.toNumber() < 0) {
        averagePrice.multiply(-1);
      }
    }
    rowData.push(averagePrice);
    // totalPnL, unrealizedPnL, realizedPnL
    let totalPnL = null;
    let unrealizedPnL = null;
    let grossPnL = data.inventory.gross_profit_and_loss;
    let fees = data.inventory.fees;
    if (data.unrealized_profit_and_loss.is_initialized) {
      unrealizedPnL = data.unrealized_profit_and_loss.value;
      totalPnL = unrealizedPnL.add(grossPnL).subtract(fees);
    }
    rowData.push(totalPnL);
    rowData.push(unrealizedPnL);
    rowData.push(grossPnL.subtract(fees));
    // fees
    rowData.push(fees);
    // cost Basis
    rowData.push(data.inventory.position.cost_basis);
    // currency
    rowData.push(data.inventory.position.key.currency);
    // volume
    rowData.push(data.inventory.volume);
    // transaction count
    rowData.push(data.inventory.transaction_count);

    return rowData;
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(payload);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.handleDataRemove(payload);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(payload);
    }
  }

  /** @private */
  handleDataAdd(payload) {
    let rowIndex = payload;
    let account = this.indexedModel.getValueAt(0, rowIndex);
    let totalPnL = this.indexedModel.getValueAt(5, rowIndex) || Money.fromNumber(0);
    let unrealizedPnL = this.indexedModel.getValueAt(6, rowIndex) || Money.fromNumber(0);
    let fees = this.indexedModel.getValueAt(8, rowIndex) || Money.fromNumber(0);
    let currencyId = this.indexedModel.getValueAt(10, rowIndex);

    if (!this.accountTotals.has(account.id)) {
      this.accountTotals.set(account.id, {
        totalPnL: Money.fromNumber(0),
        unrealizedPnL: Money.fromNumber(0),
        fees: Money.fromNumber(0),
        count: 0
      });
    }

    this.addToAccountTotals(account, totalPnL, unrealizedPnL, fees, currencyId);
    this.accountTotals.get(account.id).count++;
    this.signalManager.emitSignal(DataChangeType.ADD, rowIndex);
  }

  /** @private */
  handleDataRemove(payload) {
    let row = payload.row.slice();
    row = this.appendAccountTotals(row);
    let account = row[0];

    if (this.accountTotals.get(account.id).count == 1) {
      this.accountTotals.remove(account.id);
    } else {
      this.accountTotals.get(account.id).count--;
      let totalPnL = row[5];
      let unrealizedPnL = row[6];
      let fees = row[8];
      let currencyId = row[10];
      this.subtractFromAccountTotals(account, totalPnL, unrealizedPnL, fees, currencyId);
    }

    this.signalManager.emitSignal(DataChangeType.REMOVE, {
      index: payload.index,
      row: Object.freeze(row)
    });
  };

  /** @private */
  handleDataUpdate(payload) {
    let original = payload.original.slice();
    original = this.appendAccountTotals(original);
    let rowIndex = payload.index;
    let account = original[0];

    // calculate deltas
    let totalPnLDelta = this.indexedModel.getValueAt(5, rowIndex);
    if (original[5] != null) {
      totalPnLDelta = totalPnLDelta.subtract(original[5]);
    }
    let unrealizedPnLDelta = this.indexedModel.getValueAt(6, rowIndex);
    if (original[6] != null) {
      unrealizedPnLDelta = unrealizedPnLDelta.subtract(original[6]);
    }
    let feesDelta = this.indexedModel.getValueAt(8, rowIndex);
    if (original[8] != null) {
      feesDelta = feesDelta.subtract(original[8]);
    }
    let currencyId = original[10];
    this.addToAccountTotals(account, totalPnLDelta, unrealizedPnLDelta, feesDelta, currencyId);
    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: rowIndex,
      original: Object.freeze(original)
    });
  }

  /** @private */
  appendAccountTotals(row) {
    let accountTotals = this.accountTotals.get(row[0].id);
    row.push(accountTotals.totalPnL);
    row.push(accountTotals.unrealizedPnL);
    row.push(accountTotals.fees);
    return row;
  }

  /** @private **/
  addToAccountTotals(account, totalPnL, unrealizedPnL, fees, currencyId) {
    // currency conversion
    totalPnL = this.convertCurrencies(currencyId, this.baseCurrencyId, totalPnL);
    unrealizedPnL = this.convertCurrencies(currencyId, this.baseCurrencyId, unrealizedPnL);
    fees = this.convertCurrencies(currencyId, this.baseCurrencyId, fees);

    let accountTotals = this.accountTotals.get(account.id);
    if (totalPnL != null) {
      accountTotals.totalPnL = accountTotals.totalPnL.add(totalPnL);
    }
    if (unrealizedPnL != null) {
      accountTotals.unrealizedPnL = accountTotals.unrealizedPnL.add(unrealizedPnL);
    }
    accountTotals.fees = accountTotals.fees.add(fees);
  }

  /** @private */
  subtractFromAccountTotals(account, totalPnL, unrealizedPnL, fees, currencyId) {
    // currency conversion
    totalPnL = this.convertCurrencies(currencyId, this.baseCurrencyId, totalPnL);
    unrealizedPnL = this.convertCurrencies(currencyId, this.baseCurrencyId, unrealizedPnL);
    fees = this.convertCurrencies(currencyId, this.baseCurrencyId, fees);

    let accountTotals = this.accountTotals.get(account.id);
    accountTotals.totalPnL = accountTotals.totalPnL.subtract(totalPnL);
    accountTotals.unrealizedPnL = accountTotals.unrealizedPnL.subtract(unrealizedPnL);
    accountTotals.fees = accountTotals.fees.subtract(fees);
  }

  /** @private */
  convertCurrencies(fromCurrencyId, toCurrencyId, amount) {
    return this.exchangeRateTable.convert(
      amount,
      fromCurrencyId,
      toCurrencyId
    );
  }
}
