import View from './view';
import TableModel from './table-model';

class Controller {
  constructor(react, componentModel) {
    this.componentModel = componentModel;
    this.tableModel = new TableModel(this.componentModel.dataModel);
    this.componentModel.setReference(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
    this.view.setTableModel(this.tableModel);
  }

  componentDidMount() {
    this.renderInterval = setInterval(this.view.render, 500);
  }

  componentWillUnmount() {
    clearInterval(this.renderInterval);
    this.view.dispose();
  }

  rowUpdate(index) {
    this.tableModel.rowUpdate(index);
  }

  rowAdd(index) {
    this.tableModel.rowAdd(index);
  }

  rowRemove(index) {
    this.tableModel.rowRemove(index);
  }

  rowMove(fromIndex, toIndex) {
    this.tableModel.rowMove(fromIndex, toIndex);
  }
}

export default Controller;
