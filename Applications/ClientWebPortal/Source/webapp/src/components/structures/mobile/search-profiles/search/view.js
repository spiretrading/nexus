import './style.scss';
import React from 'react';
import PrimaryButton from 'components/reusables/common/primary-button';
import ProfileSearchPanel from 'components/reusables/common/profile-search-panel';
import CommonView from 'components/structures/common/search-profiles/search/common-view';

class View extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let newAccountBtnModel = {
      label: 'New Account'
    };

    let newGroupModel = {
      label: 'New Group'
    };

    let onNewAccountClick = this.onNewAccountClick.bind(this);
    let onNewGroupClick = this.onNewGroupClick.bind(this);
    let onSearchInputChange = this.onSearchInputChange.bind(this);

    let navigateToProfile = this.controller.navigateToProfile.bind(this.controller);
    let panels = [];
    if (this.componentModel.groupedAccounts != null) {
      for (let i=0; i<this.componentModel.groupedAccounts.length; i++) {
        let panelModel = {
          groupName: this.componentModel.groupedAccounts[i].name,
          searchString: this.componentModel.searchString
        };
        panels.push(
          <ProfileSearchPanel key={i} model={panelModel} navigateToProfile={navigateToProfile}/>
        );
      }
    }

    let newGroupCreateBtnModel = {
      label: 'Create'
    };

    let newGroupCancelBtnModel = {
      label: 'Cancel'
    };

    return (
      <div id="search-profiles-container">
        <div className="search-profiles-wrapper">
          <div className="menu-info-wrapper">
            <PrimaryButton className="new-account-btn" model={newAccountBtnModel} onClick={onNewAccountClick}/>
            <PrimaryButton className="new-group-btn" model={newGroupModel} onClick={onNewGroupClick}/>
          </div>
          <div className="search-wrapper">
            <div className="search-input-wrapper">
              <input className="search-input" onChange={onSearchInputChange}/>
              <span className="icon-magnifying_glass"></span>
            </div>
            <div className="search-results-wrapper">
              {panels}
            </div>
          </div>

          <div id="new-group-modal" className="modal fade" tabIndex="-1" role="dialog">
            <div className="modal-dialog" role="document">
              <div className="modal-content">
                <div className="modal-header">
                  Create Group
                </div>
                <div className="modal-body change-picture-wrapper">
                  <input type="text" defaultValue="" className="name-input" onKeyPress={this.onGroupNamePress.bind(this)}/>
                  <div className='buttons-wrapper'>
                    <PrimaryButton model={newGroupCreateBtnModel}
                                   className="create-button"
                                   onClick={this.onNewGroupCreateClick.bind(this)}/>
                    <PrimaryButton model={newGroupCancelBtnModel}
                                   className="cancel-button"
                                   onClick={this.onNewGroupCancelClick.bind(this)}/>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

export default View;
