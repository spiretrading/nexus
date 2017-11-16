import Model from 'utils/table-models/model';
import ViewData from 'utils/table-models/view-model/view-data';
import DefaultStyleRule from 'utils/table-models/default-style-rule';
import numberFormatter from 'utils/number-formatter';
import DataChangeType from 'utils/table-models/model/data-change-type';
import SignalManager from 'utils/signal-manager';

export default class extends Model {
  constructor(sourceModel) {
    super();
    this.sourceModel = sourceModel;
    this.defaultStyle = new DefaultStyleRule();
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.toViewData = this.toViewData.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnName(columnIndex) {
    return this.sourceModel.getColumnName(columnIndex);
  }

  getValueAt(x, y) {
    let value = this.sourceModel.getValueAt(x, y);
    return this.toViewData(value);
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

  /** @protected */
  toViewData(value) {
    let display = this.getDisplay(value);
    return new ViewData(value, display, this.defaultStyle);
  }

 /** @private */
  getDisplay(value) {
    if (value == null) {
      return 'N/A';
    } else if (typeof value === 'number' && !Number.isNaN(value)) {
      return numberFormatter.formatWithComma(value);
    } else if (typeof value === 'string') {
      return value;
    } else {
      return value.toString();
    }
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.REMOVE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        row: Object.freeze(payload.row.map(this.toViewData))
      });
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        original: Object.freeze(payload.original.map(this.toViewData))
      });
    } else {
      this.signalManager.emitSignal(dataChangeType, payload);
    }
  }
}
