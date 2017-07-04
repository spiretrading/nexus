import './style.scss';
import React from 'react';
import CommonView from 'components/reusables/common/new-personal-details/common-view';
import definitionsService from 'services/definitions';

class DesktopView extends CommonView {
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
    this.onChange();
  }

  render() {
    let countryInput;
    let countryOptions = [];
    let countries = definitionsService.getCountries();
    for (let i=0; i<countries.length; i++) {
      let country = countries[i];
      countryOptions.push(
        <option key={i} value={country.code}>{country.name}</option>
      );
    }

    countryInput =
      <div className="personal-detail-row editable">
        <select className="country-input" defaultValue={this.componentModel.country} onChange={this.onChange.bind(this)}>
          {countryOptions}
        </select>
      </div>;

    return (
        <div id={this.componentModel.componentId} className="personal-details-container">
          <table className="details-container">
            <tbody>
              <tr>
                <td>First Name</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Last Name</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Username</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="user-name-input" type="text" defaultValue={this.componentModel.userName}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Role(s)</td>
                <td>
                  <div className="personal-detail-row">
                    <span title="Trader" className="role icon icon-trader" onClick={this.onRoleClick.bind(this)}/>
                    <span title="Manager" className="role icon icon-manager" onClick={this.onRoleClick.bind(this)}/>
                    <span title="Admin" className="role icon icon-admin" onClick={this.onRoleClick.bind(this)}/>
                    <span title="Service" className="role icon icon-service" onClick={this.onRoleClick.bind(this)}/>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Group(s)</td>
                <td>
                  <div className="personal-detail-row groups">
                    <ul className="group-tags"></ul>
                  </div>
                </td>
              </tr>
              <tr>
                <td>E-mail</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="email-input" type="text" defaultValue={this.componentModel.email}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Address</td>
                <td>
                  <div className="personal-detail-row editable">
                    <textarea defaultValue="" rows="1" className="address-input"/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>City</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="city-input" type="text" defaultValue={this.componentModel.city}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Province/State</td>
                <td>
                  <div className="personal-detail-row editable">
                    <input className="province-input" type="text" defaultValue={this.componentModel.province}
                           onChange={this.onChange.bind(this)}/>
                    <div className="edit-icon-wrapper">
                      <span className="icon-edit"/>
                    </div>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Country</td>
                <td>{countryInput}</td>
              </tr>
            </tbody>
          </table>
        </div>
    );
  }
}

export default DesktopView;
