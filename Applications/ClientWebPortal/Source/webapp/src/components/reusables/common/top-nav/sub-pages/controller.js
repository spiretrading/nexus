import View from './view';
import {browserHistory} from 'react-router/es6';
import profileContext from 'components/structures/common/profile/context';

class Controller {
  constructor(react) {
    this.componentModel = react.props.model || [];
    this.view = new View(react, this, this.componentModel);
  }

  /** @private */
  getParameters(path) {
    if (path.indexOf('profile') >= 0) {
      return profileContext.get().directoryEntry;
    }
  }

  componentWillUpdate(model) {
    this.componentModel = model;
    this.view.setComponentModel(this.componentModel);
  }

  getView() {
    return this.view;
  }

  navigateTo(path) {
    let parameters = this.getParameters(path);
    browserHistory.push(path, parameters);
  }
}

export default Controller;
