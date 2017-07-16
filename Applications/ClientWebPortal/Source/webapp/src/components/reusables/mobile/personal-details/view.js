import './style.scss';
import React from 'react';
import moment from 'moment';
import CommonView from 'components/reusables/common/personal-details/common-view';
import definitionsService from 'services/definitions';

class MobileView extends CommonView {
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

    let details;
    if (this.componentModel.isReadOnly) {
      let countryName = definitionsService.getCountryName(this.componentModel.country);

      details =
        <div className="details-container">
          <div className="personal-detail-row">
            <div className="personal-detail-label">First Name</div>
            <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Last Name</div>
            <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Username</div>
            {this.componentModel.userName}
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Role(s)</div>
            <span title="Trader" className={traderClass}/>
            <span title="Manager" className={managerClass}/>
            <span title="Admin" className={adminClass}/>
            <span title="Service" className={serviceClass}/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Registration Date</div>
            {registrationDate}
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">ID</div>
            {this.componentModel.id}
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">E-mail</div>
            <input className="email-input" type="text" defaultValue={this.componentModel.email}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Address</div>
            <textarea defaultValue="" rows="1" className="address-input" readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">City</div>
            <input className="city-input" type="text" defaultValue={this.componentModel.city}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Province/State</div>
            <input className="province-input" type="text" defaultValue={this.componentModel.province}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Country</div>
            <input className="country-input" type="text" defaultValue={countryName}
                   readOnly/>
          </div>

        </div>
    } else {
      let countryOptions = [];
      let countries = definitionsService.getCountries();
      for (let i=0; i<countries.length; i++) {
        let country = countries[i];
        countryOptions.push(
          <option key={i} value={country.code}>{country.name}</option>
        );
      }

      details =
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
            {this.componentModel.userName}
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Role(s)</div>
            <span title="Trader" className={traderClass} onClick={this.onRoleClick.bind(this)}/>
            <span title="Manager" className={managerClass} onClick={this.onRoleClick.bind(this)}/>
            <span title="Admin" className={adminClass} onClick={this.onRoleClick.bind(this)}/>
            <span title="Service" className={serviceClass} onClick={this.onRoleClick.bind(this)}/>
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">Registration Date</div>
            {registrationDate}
          </div>

          <div className="personal-detail-row">
            <div className="personal-detail-label">ID</div>
            {this.componentModel.id}
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
    }

    return (
        <div id={this.componentModel.componentId} className="personal-details-container">
          {details}
        </div>
    );
  }
}

export default MobileView;
