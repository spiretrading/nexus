class RouteParameters {
  constructor() {
    this.parameters;
  }

  set(parameters) {
    this.parameters = parameters;
  }

  get() {
    return this.parameters;
  }
}

export default new RouteParameters();