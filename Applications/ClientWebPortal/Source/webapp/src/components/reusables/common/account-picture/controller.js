import View from './view';

class Controller {
  constructor(react) {
    this.componentModel = react.props.model || {};
    this.onPictureChange = react.props.onPictureChange;
    this.view = new View(react, this, this.componentModel);
  }

  getView() {
    return this.view;
  }

  componentWillUpdate(model) {
    this.componentModel = model;
    this.view.setComponentModel(this.componentModel);
  }

  onAccountPictureChange(newPictureData) {
    this.onPictureChange(newPictureData);
  }
}

export default Controller;
