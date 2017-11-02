import ChainableModel from './chainable-model';

export default class extends ChainableModel {
  constructor(sourceModel) {
    super(sourceModel);
  }

  getRowCount() {
    return this.sourceModel.getRowCount();
  }

  getColumnCount() {
    return this.sourceModel.getColumnCount();
  }

  getColumnHeader(x) {
    return this.sourceModel.getColumnHeader(x);
  }

  getValueAt(x, y) {
    let cellValue = this.sourceModel.getValueAt(x, y);
    if (cellValue.value.constructor != null && cellValue.value.constructor.name == 'Money') {
      let display = cellValue.display.replace(/,/g, '');
      display = display.replace(/\$/g, '');
      return display;
    } else {
      return cellValue.display;
    }
  }
}
