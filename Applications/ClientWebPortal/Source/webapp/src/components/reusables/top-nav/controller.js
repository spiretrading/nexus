import View from './view';

class Controller {
  constructor(react) {
    this.componentModel = {};
    this.view = new View(react, this, clone(this.componentModel));
  }

  getView() {
    return this.view;
  }
}

export default Controller;