class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.navigateToTraderProfile = react.props.navigateToTraderProfile;
    this.navigateToGroupProfile = react.props.navigateToGroupProfile;
    this.loadGroupAccounts = react.props.loadAccounts;

    this.loadAccounts = this.loadAccounts.bind(this);
    this.navigateToTraderProfile = this.navigateToTraderProfile.bind(this);
    this.navigateToGroupProfile = this.navigateToGroupProfile.bind(this);
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

  navigateToTraderProfile(traderId) {
    this.navigateToTraderProfile(traderId);
  }

  navigateToGroupProfile(groupId) {
    this.navigateToGroupProfile(groupId);
  }

  loadAccounts() {
    this.loadGroupAccounts(this.componentModel.groupId);
  }
}

export default Controller;
