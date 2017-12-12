class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onSelected = react.props.onSelected;
    this.onDeselected = react.props.onDeselected;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }
}

export default Controller;
