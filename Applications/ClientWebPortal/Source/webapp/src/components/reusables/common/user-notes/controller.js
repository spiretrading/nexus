class Controller {
  constructor(react) {
    this.componentModel = clone(react.props.model);
    this.onChange = react.props.onChange;

    this.onUserNotesChange = this.onUserNotesChange.bind(this);
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    overwriteMerge(this.componentModel, model);
    this.view.setComponentModel(this.componentModel);
  }

  onUserNotesChange(newNotes) {
    this.onChange(newNotes);
  }
}

export default Controller;
