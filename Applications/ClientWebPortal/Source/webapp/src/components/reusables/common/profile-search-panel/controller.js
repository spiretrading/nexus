class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.navigateToProfile = react.props.navigateToProfile;
    this.loadGroupAccounts = react.props.loadAccounts;
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

  navigateToAccountProfile(traderId) {
    this.navigateToProfile(traderId);
  }

  loadAccounts() {
    this.loadGroupAccounts(this.componentModel.groupId);
  }
}

export default Controller;
