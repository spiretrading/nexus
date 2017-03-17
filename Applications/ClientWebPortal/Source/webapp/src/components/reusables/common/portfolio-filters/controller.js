class Controller {
  constructor(react, componentModel) {
    this.componentModel = componentModel;
    this.onParametersSave = react.props.onSave;
    this.isInitialized = false;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  initializeColumns() {
    this.componentModel.columns = [
      { id: 1, name: 'Group' },
      { id: 2, name: 'Account Total P&L' },
      { id: 3, name: 'Account Unrealized' },
      { id: 4, name: 'Account Realized' },
      { id: 5, name: 'Account Fees' },
      { id: 6, name: 'Open Quantity' },
      { id: 7, name: 'Side' },
      { id: 8, name: 'Average Price' },
      { id: 9, name: 'Total P&L' },
      { id: 10, name: 'Unrealized' },
      { id: 11, name: 'Realized' },
      { id: 12, name: 'Fees' },
      { id: 13, name: 'Cost Basis' },
      { id: 14, name: 'Volume' },
      { id: 15, name: 'Trades' }
    ];
  }

  componentWillUpdate(model) {
    if (model != null) {
      if (!this.isInitialized) {
        this.isInitialized = true;
        overwriteMerge(this.componentModel, model);
        this.componentModel.filter = {
          columns: [],
          currencies: [],
          groups: [],
          markets: []
        };
        this.initializeColumns();
      }

      this.view.setComponentModel(this.componentModel);
    }
  }

  onFilterChange(category, availableItems, selectedItems) {
    if (category === 'Group') {
      this.componentModel.groups = availableItems;
      this.componentModel.filter.groups = selectedItems;
    } else if (category === 'Currency') {
      this.componentModel.currencies = availableItems;
      this.componentModel.filter.currencies = selectedItems;
    } else if (category === 'Market') {
      this.componentModel.markets = availableItems;
      this.componentModel.filter.markets = selectedItems;
    } else if (category === 'Column') {
      this.componentModel.columns = availableItems;
      this.componentModel.filter.columns = selectedItems;
    }
    this.view.update(this.componentModel);
  }

  update() {
    this.onParametersSave(this.componentModel.filter);
  }
}

export default Controller;
