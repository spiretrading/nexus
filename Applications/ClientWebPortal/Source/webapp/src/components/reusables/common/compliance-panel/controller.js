import {MarketDataServiceClient} from 'spire-client';

class Controller {
  constructor(react, componentModel) {
    this.componentModel = clone(componentModel);
    this.onUpdated = react.props.onUpdate;
    this.marketDataServiceClient = new MarketDataServiceClient();
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  componentWillUpdate(model) {
    if (model != null) {
      overwriteMerge(this.componentModel, model);
      this.view.setComponentModel(this.componentModel);
    }
  }

  componentDidMount() {
    this.view.initialize();
  }

  onParameterUpdated(parameterName, value) {
    let parameters = this.componentModel.schema.parameters;
    for (let i=0; i<parameters.length; i++) {
      if (parameters[i].name == parameterName) {
        parameters[i].value.value = value;
      }
    }
    this.onUpdated(
      this.componentModel.ruleEntryId,
      this.componentModel.schema.parameters,
      this.componentModel.state
    );
  }

  onStatusChange(newValue) {
    this.componentModel.status = newValue;
    this.onUpdated(
      this.componentModel.ruleEntryId,
      this.componentModel.parameters,
      this.componentModel.status
    );
  }

  searchSymbols(input) {
    return this.marketDataServiceClient.loadSecurityInfoFromPrefix(input);
  }
}

export default Controller;
