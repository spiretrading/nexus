import View from './view';

class Controller {
  constructor(react) {
    this.componentModel = react.props.model || [];
    this.view = new View(react, this, cloneObject(this.componentModel));
  }

  componentWillUpdate(nextProps, nextState) {
    this.componentModel = nextProps.model;
    this.view.setComponentModel(this.componentModel);
  }

  getView() {
    return this.view;
  }
}

export default Controller;