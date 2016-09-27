class Controller {
  constructor(react, componentModel) {
    this.componentModel = cloneObject(componentModel);
    this.onChange = react.props.onChange;
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

  componentWillUnmount() {
    this.view.dispose.apply(this.view);
  }

  onSelectionChange(newValue) {
    this.onChange(newValue);
  }
}

export default Controller;
