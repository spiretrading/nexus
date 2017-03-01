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
      { id: 2, name: 'Account' },
      { id: 3, name: 'Account Total P&L' },
      { id: 4, name: 'Account Unrealized' },
      { id: 5, name: 'Account Realized' },
      { id: 6, name: 'Account Fees' },
      { id: 7, name: 'Security' },
      { id: 8, name: 'Open Quantity' },
      { id: 9, name: 'Side' },
      { id: 10, name: 'Average Price' },
      { id: 11, name: 'Total P&L' },
      { id: 12, name: 'Unrealized' },
      { id: 13, name: 'Realized' },
      { id: 14, name: 'Fees' },
      { id: 15, name: 'Cost Basis' },
      { id: 16, name: 'Currency' },
      { id: 17, name: 'Currency' },
      { id: 18, name: 'Volume' },
      { id: 19, name: 'Trades' }
    ];
  }

  componentWillUpdate(model) {
    if (model != null) {
      if (!this.isInitialized) {
        this.isInitialized = true;
        overwriteMerge(this.componentModel, model);
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
    // TODO: update parameters to back-end when the API is ready
  }
}

export default Controller;
