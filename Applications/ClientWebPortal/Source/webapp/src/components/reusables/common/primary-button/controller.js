class Controller {
  constructor(react, componentModel) {
    this.componentModel = cloneObject(componentModel);
    this.onClick = react.props.onClick;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      $.extend(true, this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  onButtonClick() {
    this.onClick();
  }
}

export default Controller;
