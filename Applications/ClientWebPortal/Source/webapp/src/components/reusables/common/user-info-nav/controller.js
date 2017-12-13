import View from './view';

/** User info nav bar controller */
class Controller {
  constructor(react) {
    this.componentModel = react.props.model || {};
    this.view = new View(react, this, this.componentModel);
  }

  getView() {
    return this.view;
  }

  componentWillUpdate(model) {
    overwriteMerge(this.componentModel, model);
    this.view.setComponentModel(this.componentModel);
  }
}

export default Controller;
