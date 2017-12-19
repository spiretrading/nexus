import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import AccModRequest from 'components/reusables/common/account-modification-request';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  componentDidUpdate() {
    $('#modification-request-history-container').fadeIn({
      duration: Config.FADE_DURATION
    });
  }

  render() {
    let content;

    if (this.controller.isRequiredDataLoaded()) {
      let requestPanels = [];
      for (let i=0; i<this.componentModel.accountModificationRequests.length; i++) {
        requestPanels.push(
          <div key={i} className="request-wrapper">
            <AccModRequest
              request = {this.componentModel.accountModificationRequests[i].request}
              update = {this.componentModel.accountModificationRequests[i].update}
              onClick = {this.controller.onSelect}
            />
          </div>
        );
      }
      content =
        <div>
          <div className="page-top-header row">
            Request History
          </div>
          <div className="requests-wrapper">
            {requestPanels}
          </div>
        </div>
    }

    return (
      <div id="modification-request-history-container">
        {content}
      </div>
    );
  }
}

export default View;
