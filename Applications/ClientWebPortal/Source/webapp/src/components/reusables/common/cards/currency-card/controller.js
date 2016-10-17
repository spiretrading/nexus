class Controller {
  constructor(react, componentModel) {
    this.componentModel = cloneObject(componentModel);
    this.onChange = react.props.onChange;
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      $.extend(true, this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  componentDidMount() {
    this.view.initialize.apply(this.view);
    this.currencySelectedListenerId = EventBus.subscribe(Event.Profile.RiskControls.CURRENCY_SELECTED, () => {
      this.view.enable.apply(this.view);
    });
  }

  componentWillUnmount() {
    this.view.dispose.apply(this.view);
    EventBus.unsubscribe(this.currencySelectedListenerId);
  }

  onAmountChange(newAmount) {
    this.onChange(newAmount);
  }
}

export default Controller;
