class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onSelected = react.props.onSelected;
    this.onDeselected = react.props.onDeselected;
    this.onRequestClick = react.props.onClick;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  onClick() {
    this.onRequestClick(
      this.componentModel.request.id,
      this.componentModel.request.requestType,
      this.componentModel.request.submissionAccount,
      this.componentModel.request.account,
      this.componentModel.update.status
    );
  }
}

export default Controller;
