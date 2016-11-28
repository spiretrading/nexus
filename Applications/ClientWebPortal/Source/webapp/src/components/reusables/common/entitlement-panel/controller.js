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

  componentWillUpdate(model) {
    if (model != null) {
      $.extend(true, this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  onEntitlementSelected() {
    let id = this.componentModel.entitlement.group_entry.id;
    this.onSelected(id);
  }

  onEntitlementDeselected() {
    let id = this.componentModel.entitlement.group_entry.id;
    this.onDeselected(id);
  }
}

export default Controller;
