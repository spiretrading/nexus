class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.submit = react.props.onSubmit;
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

  cancel(onCancelled) {
    this.submit(false, null, onCancelled);
  }

  save(newPictureData) {
    if (newPictureData === '') {
      this.submit(false, null);
    } else {
      this.submit(true, newPictureData);
    }
  }
}

export default Controller;
