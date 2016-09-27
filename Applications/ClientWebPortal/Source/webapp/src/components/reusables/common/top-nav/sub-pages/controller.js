import View from './view';
import {browserHistory} from 'react-router/es6';

class Controller {
  constructor(react) {
    this.componentModel = react.props.model || [];
    this.view = new View(react, this, this.componentModel);
  }

  componentWillUpdate(model) {
    this.componentModel = model;
    this.view.setComponentModel(this.componentModel);
  }

  getView() {
    return this.view;
  }

  navigateTo(path) {
    browserHistory.push(path);
  }
}

export default Controller;
