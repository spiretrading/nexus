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
    overwriteMerge(this.componentModel, model);
    this.view.setComponentModel(this.componentModel);
  }

  componentWillUnmount() {
    this.view.dispose();
  }

  onAccountPictureChange(newPictureData) {
    this.onPictureChange(newPictureData);
  }
}

export default Controller;
