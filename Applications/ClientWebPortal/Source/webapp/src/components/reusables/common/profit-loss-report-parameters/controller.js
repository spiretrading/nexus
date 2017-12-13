class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onGenerate = react.props.onGenerate;

    this.generate = this.generate.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentDidMount() {
    this.view.initialize();
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  generate(startDate, endDate) {
    this.onGenerate(startDate, endDate);
  }
}

export default Controller;
