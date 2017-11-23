import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';
import definitionsService from 'services/definitions';
import { Money } from 'spire-client';

export default class extends Model {
  constructor(sourceModel, baseCurrencyId) {
    super();
    this.sourceModel = sourceModel;
    this.baseCurrencyId = baseCurrencyId;
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
    this.exchangeRateTable = definitionsService.getExchangeRateTable();

    this.totals = [];

    this.initialize();
  }

  getRowCount() {
    return 1;
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(x) {
    return this.sourceModel.getColumnName(x);
  }

  getValueAt(x, y) {
    if (y != 0) {
      throw new RangeError('There is only ever one row. Received: ' + y);
    }
    return this.totals[x];
  }

  addDataChangeListener(listener) {
    return this.signalManager.addListener(listener);
  }

  removeDataChangeListener(subId) {
    this.signalManager.removeListener(subId);
  }

  dispose() {
    this.sourceModel.removeDataChangeListener(this.dataChangeSubId);
  }

  /** @private */
  initialize() {
    this.totals[0] = this.baseCurrencyId;
    let rowCount = this.sourceModel.getRowCount();
    if (rowCount > 0) {
      for (let i=0; i<rowCount; i++) {
        this.addSourceData(i);
      }
    } else {
      let columnCount = this.sourceModel.getColumnCount();
      for (let i=1; i<columnCount; i++) {
        this.totals.push(null);
      }
    }
  }

  /** @private */
  addSourceData(rowIndex) {
    let columnCount = this.sourceModel.getColumnCount();
    for (let i=1; i<columnCount; i++) {
      let value = this.sourceModel.getValueAt(i, rowIndex);
      this.addToTotals(rowIndex, i, value);
    }
  }

  /** @private */
  addToTotals(rowIndex, columnIndex, value) {
    if (value instanceof Money) {
      let currencyId = this.sourceModel.getValueAt(0, rowIndex);
      value = this.convertCurrencies(currencyId, this.baseCurrencyId, value);
    }

    if (this.totals[columnIndex] == null) {
      this.totals[columnIndex] = value;
    } else {
      if (value == null) {
        return;
      }

      if (value instanceof Money) {
        this.totals[columnIndex] = this.totals[columnIndex].add(value);
      } else {
        this.totals[columnIndex] = this.totals[columnIndex] + value;
      }
    }
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.handleDataAdd(payload);
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.handleDataUpdate(payload.index, payload.original);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.handleDataRemove(payload.index, payload.row);
    }
  }

  /** @private */
  handleDataAdd(rowIndex) {
    let original = this.totals.slice();
    this.addSourceData(rowIndex);
    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: rowIndex,
      original: Object.freeze(original)
    });
  }

  /** @private */
  handleDataUpdate(rowIndex, originalRow) {
    let originalTotals = this.totals.slice();
    let columnCount = this.sourceModel.getColumnCount();
    for (let i=1; i<this.totals.length; i++) {
      let newValue = this.sourceModel.getValueAt(i, rowIndex);
      let delta;
      let lastValue = originalRow[i];
      if (newValue instanceof Money) {
        delta = newValue.subtract(lastValue);
      } else {
        delta = newValue - lastValue;
      }
      this.addToTotals(rowIndex, i, delta);
    }

    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: 0,
      original: Object.freeze(originalTotals)
    });
  }

  /** @private */
  handleDataRemove(rowIndex, removedRow) {
    let originalTotals = this.totals.slice();
    for (let i=1; i<this.totals.length; i++) {
      let delta;
      if (this.totals[i] instanceof Money) {
        delta = removedRow[i].multiply(-1);
      } else {
        delta = removedRow[i] * -1;
      }
      this.addToTotals(rowIndex, i, delta);
    }

    this.signalManager.emitSignal(DataChangeType.UPDATE, {
      index: 0,
      original: Object.freeze(originalTotals)
    });
  }

  /** @private */
  convertCurrencies(fromCurrencyId, toCurrencyId, amount) {
    if (fromCurrencyId.toNumber() != toCurrencyId.toNumber()) {
      return this.exchangeRateTable.convert(
        amount,
        fromCurrencyId,
        toCurrencyId
      );
    } else {
      return amount;
    }
  }
}
