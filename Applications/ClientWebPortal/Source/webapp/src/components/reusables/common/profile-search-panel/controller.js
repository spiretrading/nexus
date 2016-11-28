class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.navigateToProfile = react.props.navigateToProfile;
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
      $.extend(true, this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  navigateToAccountProfile(traderId) {
    this.navigateToProfile(traderId);
  }
}

export default Controller;
