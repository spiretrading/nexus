class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.navigateToProfile = react.props.navigateToProfile;
    this.getGroupAccounts = react.props.getAccounts;
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

  getAccounts() {
    this.getGroupAccounts(this.componentModel.groupId);
  }
}

export default Controller;
