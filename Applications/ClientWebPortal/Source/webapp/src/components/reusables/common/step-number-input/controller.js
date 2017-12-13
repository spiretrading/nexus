class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onChange = react.props.onChange;
    this.onInputValidationFail = react.props.onInputValidationFail;

    this.setView = this.setView.bind(this);
    this.onValueChange = this.onValueChange.bind(this);
    this.onValidationFail = this.onValidationFail.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    this.view.initialize();
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

  onValueChange(newValue) {
    this.onChange(newValue);
  }

  onValidationFail(failMessage) {
    if (this.onInputValidationFail != null) {
      this.onInputValidationFail(failMessage);
    }
  }
}

export default Controller;
