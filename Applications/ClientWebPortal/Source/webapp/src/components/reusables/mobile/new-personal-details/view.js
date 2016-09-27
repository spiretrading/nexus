import './style.scss';
import React from 'react';
import CommonView from 'components/reusables/common/new-personal-details/common-view';
import definitionsService from 'services/definitions';

class MobileView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  /** @private */
  onRoleClick(event) {
    let $icon = $(event.currentTarget);
    if ($icon.hasClass('active')) {
      $icon.removeClass('active');
    } else {
      $icon.addClass('active');
    }
    this.onChange.apply(this);
  }

  render() {
    let countryOptions = [];
    let countries = definitionsService.getCountries();
    for (let i=0; i<countries.length; i++) {
      let country = countries[i];
      countryOptions.push(
        <option key={i} value={country.code}>{country.name}</option>
      );
    }

    let details =
      <div className="details-container">
        <div className="personal-detail-row editable">
          <div className="personal-detail-label">First Name</div>
          <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">Last Name</div>
          <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row">
          <div className="personal-detail-label">Username</div>
          <input className="user-name-input" type="text" defaultValue={this.componentModel.firstName}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row">
          <div className="personal-detail-label">Role(s)</div>
          <span title="Trader" className="role icon icon-trader" onClick={this.onRoleClick.bind(this)}/>
          <span title="Manager" className="role icon icon-manager" onClick={this.onRoleClick.bind(this)}/>
          <span title="Admin" className="role icon icon-admin" onClick={this.onRoleClick.bind(this)}/>
          <span title="Service" className="role icon icon-service" onClick={this.onRoleClick.bind(this)}/>
        </div>

        <div className="personal-detail-row">
          <div className="personal-detail-label">Group(s)</div>
          <ul className="group-tags"></ul>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">E-mail</div>
          <input className="email-input" type="text" defaultValue={this.componentModel.email}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">Address</div>
          <textarea defaultValue="" rows="1" className="address-input"/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">City</div>
          <input className="city-input" type="text" defaultValue={this.componentModel.city}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">Province/State</div>
          <input className="province-input" type="text" defaultValue={this.componentModel.province}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>

        <div className="personal-detail-row editable">
          <div className="personal-detail-label">Country</div>
          <select className="country-input" defaultValue={this.componentModel.country} onChange={this.onChange.bind(this)}>
            {countryOptions}
          </select>
        </div>
      </div>

    return (
        <div id={this.componentModel.componentId} className="personal-details-container">
          {details}
        </div>
    );
  }
}

export default MobileView;
