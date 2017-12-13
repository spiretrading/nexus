import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';
import { Money } from 'spire-client';

export default class extends Model {
  constructor(sourceModel) {
    super();
    this.sourceModel = sourceModel;
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(x) {
    return this.sourceModel.getColumnName(x);
  }

  getValueAt(x, y) {
    let cellValue = this.sourceModel.getValueAt(x, y);
    return this.transform(cellValue);
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
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.signalManager.emitSignal(dataChangeType, payload);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        row: Object.freeze(payload.row.map(this.transform))
      });
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        original: Object.freeze(payload.original.map(this.transform))
      });
    }
  }

  /** @private */
  transform(cellValue) {
    if (cellValue.value.constructor != null && cellValue.value instanceof Money) {
      let display = cellValue.display.replace(/,/g, '');
      display = display.replace(/\$/g, '');
      return display;
    } else {
      return cellValue.display;
    }
  }
}
