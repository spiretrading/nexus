import Model from 'utils/table-models/model';
import SignalManager from 'utils/signal-manager';
import DataChangeType from 'utils/table-models/model/data-change-type';

export default class extends Model {
  constructor(sourceModel, columnsToOmit) {
    super();
    this.sourceModel = sourceModel;
    this.signalManager = new SignalManager();
    this.onDataChange = this.onDataChange.bind(this);
    this.dataChangeSubId = this.sourceModel.addDataChangeListener(this.onDataChange);

    this.omittedCount = 0;
    this.projectionToSourceColumns = [];

    this.initialize(columnsToOmit);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount() - this.omittedCount;
  }

  getColumnName(x) {
    return this.sourceModel.getColumnName(this.projectionToSourceColumns[x]);
  }

  getValueAt(x, y) {
    return this.sourceModel.getValueAt(this.projectionToSourceColumns[x], y);
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
  initialize(columnsToOmit) {
    this.projectionToSourceColumns = [];
    let sourceColumnsCount = this.sourceModel.getColumnCount();
    for (let x=0; x<sourceColumnsCount; x++) {
      if (columnsToOmit.includes(x)) {
        this.omittedCount++;
      } else {
        this.projectionToSourceColumns.push(x);
      }
    }
  }

  /** @private */
  onDataChange(dataChangeType, payload) {
    if (dataChangeType == DataChangeType.ADD) {
      this.signalManager.emitSignal(dataChangeType, payload);
    } else if (dataChangeType == DataChangeType.REMOVE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        row: Object.freeze(this.transformRow(payload.row))
      });
    } else if (dataChangeType == DataChangeType.UPDATE) {
      this.signalManager.emitSignal(dataChangeType, {
        index: payload.index,
        original: Object.freeze(this.transformRow(payload.original))
      });
    }
  }

  /** @private */
  transformRow(row) {
    let transformed = [];
    for (let i=0; i<this.projectionToSourceColumns.length; i++) {
      transformed.push(row[this.projectionToSourceColumns[i]]);
    }
    return transformed;
  }
}
