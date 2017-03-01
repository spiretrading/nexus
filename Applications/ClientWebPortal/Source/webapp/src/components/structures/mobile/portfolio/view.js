import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import PortfolioParameters from 'components/reusables/common/portfolio-filters';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#portfolio-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  render() {
    let parametersModel = {
      groups: this.componentModel.managedGroups || [],
      currencies: this.componentModel.currencies || [],
      markets: this.componentModel.markets || [],
      filter: {}
    }

    let onParametersSave = this.controller.saveParameters.bind(this.controller);

    return (
      <div id="portfolio-container" className="container">
        <PortfolioParameters model={parametersModel} onSave={onParametersSave}/>
      </div>
    );
  }
}

export default View;
