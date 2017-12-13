class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onChange = react.props.onChange;

    this.setView = this.setView.bind(this);
    this.componentDidUpdate = this.componentDidUpdate.bind(this);
    this.onSelectionChange = this.onSelectionChange.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  componentDidUpdate() {
    this.view.componentDidUpdate();
  }

  onSelectionChange(newValue) {
    this.onChange(newValue);
  }
}

export default Controller;
