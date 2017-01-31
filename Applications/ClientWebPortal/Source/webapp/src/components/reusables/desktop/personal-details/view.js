import './style.scss';
import React from 'react';
import moment from 'moment';
import CommonView from 'components/reusables/common/personal-details/common-view';
import definitionsService from 'services/definitions';
import HashMap from 'hashmap';

class DesktopView extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let defaultClass = 'role icon icon-';
    let traderClass = defaultClass + 'trader';
    let managerClass = defaultClass + 'manager';
    let adminClass = defaultClass + 'admin';
    let serviceClass = defaultClass + 'service';

    if (this.componentModel.roles.isTrader) {
      traderClass += ' active';
    }
    if (this.componentModel.roles.isManager) {
      managerClass += ' active';
    }
    if (this.componentModel.roles.isAdmin) {
      adminClass += ' active';
    }
    if (this.componentModel.roles.isService) {
      serviceClass += ' active';
    }

    let registrationDate = moment(this.componentModel.registrationTime, moment.ISO_8601).toDate().toLocaleDateString();

    let firstNameInput, lastNameInput, emailInput, addressInput, cityInput, provinceInput, countryInput;
    let countryOptions = [];
    let countries = definitionsService.getCountries();
    let countriesMap = new HashMap();
    for (let i=0; i<countries.length; i++) {
      let country = countries[i];
      countryOptions.push(
        <option key={i} value={country.code}>{country.name}</option>
      );
      countriesMap.set(country.code, country.name);
    }

    if (this.componentModel.isReadOnly) {
      firstNameInput =
        <div className="personal-detail-row">
          <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                 readOnly/>
        </div>;

      lastNameInput =
        <div className="personal-detail-row">
          <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                 readOnly/>
        </div>;

      emailInput =
        <div className="personal-detail-row">
          <input className="email-input" type="text" defaultValue={this.componentModel.email}
                 readOnly/>
        </div>;

      addressInput =
        <div className="personal-detail-row">
          <textarea defaultValue="" rows="1" className="address-input" readOnly/>
        </div>;

      cityInput =
        <div className="personal-detail-row">
          <input className="city-input" type="text" defaultValue={this.componentModel.city}
                 readOnly/>
        </div>;

      provinceInput =
        <div className="personal-detail-row">
          <input className="province-input" type="text" defaultValue={this.componentModel.province}
                 readOnly/>
        </div>;

      countryInput =
        <div className="personal-detail-row">
          <input className="country-input" type="text" defaultValue={countriesMap.get(this.componentModel.country)}
                 readOnly/>
        </div>;
    } else {
      firstNameInput =
        <div className="personal-detail-row editable">
          <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      lastNameInput =
        <div className="personal-detail-row editable">
          <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      emailInput =
        <div className="personal-detail-row editable">
          <input className="email-input" type="text" defaultValue={this.componentModel.email}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      addressInput =
        <div className="personal-detail-row editable">
          <textarea defaultValue="" rows="1" className="address-input"/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      cityInput =
        <div className="personal-detail-row editable">
          <input className="city-input" type="text" defaultValue={this.componentModel.city}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      provinceInput =
        <div className="personal-detail-row editable">
          <input className="province-input" type="text" defaultValue={this.componentModel.province}
                 onChange={this.onChange.bind(this)}/>
          <div className="edit-icon-wrapper">
            <span className="icon-edit"/>
          </div>
        </div>;

      countryInput =
        <div className="personal-detail-row editable">
          <div className="country-dropdown-wrapper">
            <select className="country-input" defaultValue={this.componentModel.country} onChange={this.onChange.bind(this)}>
              {countryOptions}
            </select>
            <span className="icon-arrow-down country-dropdown-arrow"></span>
          </div>
        </div>;
    }

    return (
        <div id={this.componentModel.componentId} className="personal-details-container">
          <table className="details-container">
            <tbody>
              <tr>
                <td>First Name</td>
                <td>{firstNameInput}</td>
              </tr>
              <tr>
                <td>Last Name</td>
                <td>{lastNameInput}</td>
              </tr>
              <tr>
                <td>Username</td>
                <td><div className="personal-detail-row">{this.componentModel.userName}</div></td>
              </tr>
              <tr>
                <td>Role(s)</td>
                <td>
                  <div className="personal-detail-row">
                    <span title="Trader" className={traderClass}/>
                    <span title="Manager" className={managerClass}/>
                    <span title="Admin" className={adminClass}/>
                    <span title="Service" className={serviceClass}/>
                  </div>
                </td>
              </tr>
              <tr>
                <td>Group(s)</td>
                <td></td>
              </tr>
              <tr>
                <td>Registration Date</td>
                <td><div className="personal-detail-row">{registrationDate}</div></td>
              </tr>
              <tr>
                <td>ID</td>
                <td><div className="personal-detail-row">{this.componentModel.id}</div></td>
              </tr>
              <tr>
                <td>E-mail</td>
                <td>{emailInput}</td>
              </tr>
              <tr>
                <td>Address</td>
                <td>{addressInput}</td>
              </tr>
              <tr>
                <td>City</td>
                <td>{cityInput}</td>
              </tr>
              <tr>
                <td>Province/State</td>
                <td>{provinceInput}</td>
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
