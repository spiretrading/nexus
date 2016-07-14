import View from './view';

/** Skeleton component controller */
class Controller {
  constructor(react) {
    this.componentModel = {};
    this.view = new View(react, this, cloneObject(this.componentModel));
  }

  getView() {
    return this.view;
  }
}

export default Controller;