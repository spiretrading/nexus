class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onChange = react.props.onChange;
    this.onInputValidationFail = react.props.onInputValidationFail;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    this.view.initialize.apply(this.view);
  }

  componentWillUpdate(model) {
    if (model != null) {
      $.extend(true, this.componentModel, model);
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
