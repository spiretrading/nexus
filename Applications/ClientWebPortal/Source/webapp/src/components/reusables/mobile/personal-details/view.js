import React from 'react';
import UpdatableView from 'commons/updatable-view';
import moment from 'moment';
import autosize from 'autosize';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isLastInputLineBreak = false;
  }

  /** @private */
  ignoreNewLineIfNecessary(event) {
    var numLineBreaks = (event.currentTarget.value.match(/\n/g) || []).length;
    if (event.key === 'Enter' && (numLineBreaks === 2 || this.isLastInputLineBreak || this.componentModel.addressOne.length === 0)) {
      event.preventDefault();
    }

    if (event.key === 'Backspace' || event.key === 'Enter' || event.key === 'Delete') {
      setTimeout(() => {
        this.resizeAddressLabel();
      }, 0);
    }

    if (event.keyCode == 13) {
      this.isLastInputLineBreak = true;
    } else {
      this.isLastInputLineBreak = false;
    }
  }

  /** @private */
  onAddressInput(event) {
    this.onChange();
  }

  /** private */
  onChange() {
    this.componentModel.firstName = $('#' + this.componentModel.componentId + ' .first-name-input').val().trim();
    this.componentModel.lastName = $('#' + this.componentModel.componentId + ' .last-name-input').val().trim();
    this.componentModel.email = $('#' + this.componentModel.componentId + ' .email-input').val().trim();
    this.componentModel.city = $('#' + this.componentModel.componentId + ' .city-input').val().trim();
    this.componentModel.province = $('#' + this.componentModel.componentId + ' .province-input').val().trim();
    this.componentModel.country = $('#' + this.componentModel.componentId + ' .country-input').val().trim();

    // address
    let addressLines = $('#' + this.componentModel.componentId + ' .address-input').val().trim().split('\n');
    for (let i=0; i<3; i++) {
      if (addressLines[i] != null) {
        if (i === 0) {
          this.componentModel.addressOne = addressLines[i];
        } else if (i === 1) {
          this.componentModel.addressTwo = addressLines[i];
        } else if (i === 2) {
          this.componentModel.addressThree = addressLines[i];
        }
      } else {
        if (i === 0) {
          this.componentModel.addressOne = '';
        } else if (i === 1) {
          this.componentModel.addressTwo = '';
        } else if (i === 2) {
          this.componentModel.addressThree = '';
        }
      }
    }

    this.controller.onPersonalDetailsChange(this.componentModel);
  }

  /** @private */
  initializeAddressInput() {
    let address = '';
    if (this.componentModel.addressOne.length > 0) {
      address += this.componentModel.addressOne;
    }
    if (this.componentModel.addressTwo.length > 0) {
      address += '\n' + this.componentModel.addressTwo;
    }
    if (this.componentModel.addressThree.length > 0) {
      address += '\n' + this.componentModel.addressThree;
    }
    let $addressInput = $('#' + this.componentModel.componentId + ' .address-input');
    $addressInput
      .val(address)
      .keydown(this.ignoreNewLineIfNecessary.bind(this))
      .on('input', this.onAddressInput.bind(this));
    autosize($addressInput);

    setTimeout(() => {
      let event = document.createEvent('Event');
      event.initEvent('autosize:update', true, false);
      $addressInput[0].dispatchEvent(event);
    }, 0);

    setTimeout(() => {
      this.resizeAddressLabel();
    }, 0);
  }

  /** @private */
  resizeAddressLabel() {
    let height = $('#' + this.componentModel.componentId + ' .address-input').outerHeight();
    $('#' + this.componentModel.componentId + ' .labels-container .address').outerHeight(height);
  }

  initialize() {
    this.initializeAddressInput();
  }

  dispose() {
    autosize.destroy($('#' + this.componentModel.componentId + ' .address-input'));
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

    let registrationDate = moment(this.componentModel.registrationDateTime, moment.ISO_8601).toDate().toLocaleDateString();

    let details;
    if (this.componentModel.isReadOnly) {
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
            <input className="country-input" type="text" defaultValue={this.componentModel.country}
                   readOnly/>
          </div>

        </div>
    } else {
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
            <input className="country-input" type="text" defaultValue={this.componentModel.country}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
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

export default View;