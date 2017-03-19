class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onChange = react.props.onChange;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    this.view.initialize.apply(this.view);
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  componentWillUnmount() {
    this.view.dispose();
  }

  onPersonalDetailsChange(newModel) {
    this.onChange(newModel);
  }
}

export default Controller;
