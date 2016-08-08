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
    if (event.keyCode === 13 && (numLineBreaks === 2 || this.isLastInputLineBreak || this.componentModel.addressLineOne.length === 0)) {
      event.preventDefault();
    }

    if (event.keyCode === 8 || event.keyCode === 13 || event.key === 46) {
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
    this.componentModel.country = parseInt($('#' + this.componentModel.componentId + ' .country-input').val().trim());

    // address
    let addressLines = $('#' + this.componentModel.componentId + ' .address-input').val().trim().split('\n');
    for (let i=0; i<3; i++) {
      if (addressLines[i] != null) {
        if (i === 0) {
          this.componentModel.addressLineOne = addressLines[i];
        } else if (i === 1) {
          this.componentModel.addressLineTwo = addressLines[i];
        } else if (i === 2) {
          this.componentModel.addressLineThree = addressLines[i];
        }
      } else {
        if (i === 0) {
          this.componentModel.addressLineOne = '';
        } else if (i === 1) {
          this.componentModel.addressLineTwo = '';
        } else if (i === 2) {
          this.componentModel.addressLineThree = '';
        }
      }
    }

    this.controller.onPersonalDetailsChange(this.componentModel);
  }

  /** @private */
  initializeAddressInput() {
    let address = '';
    if (this.componentModel.addressLineOne.length > 0) {
      address += this.componentModel.addressLineOne;
    }
    if (this.componentModel.addressLineTwo.length > 0) {
      address += '\n' + this.componentModel.addressLineTwo;
    }
    if (this.componentModel.addressLineThree.length > 0) {
      address += '\n' + this.componentModel.addressLineThree;
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

    let registrationDate = moment(this.componentModel.registrationTime, moment.ISO_8601).toDate().toLocaleDateString();

    let details;
    if (this.componentModel.isReadOnly) {
      details =
        <div className="details-container">
          <div className="personal-detail-row">
            <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                   readOnly/>
          </div>

          <div className="personal-detail-row">{this.componentModel.userName}</div>

          <div className="personal-detail-row">
            <span title="Trader" className={traderClass}/>
            <span title="Manager" className={managerClass}/>
            <span title="Admin" className={adminClass}/>
            <span title="Service" className={serviceClass}/>
          </div>

          <div className="personal-detail-row">{registrationDate}</div>

          <div className="personal-detail-row">{this.componentModel.id}</div>

          <div className="personal-detail-row">
            <input className="email-input" type="text" defaultValue={this.componentModel.email}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <textarea defaultValue="" rows="1" className="address-input" readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="city-input" type="text" defaultValue={this.componentModel.city}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="province-input" type="text" defaultValue={this.componentModel.province}
                   readOnly/>
          </div>

          <div className="personal-detail-row">
            <input className="country-input" type="text" defaultValue={this.componentModel.country}
                   readOnly/>
          </div>

        </div>
    } else {
      details =
        <div className="details-container">
          <div className="personal-detail-row editable">
            <input className="first-name-input" type="text" defaultValue={this.componentModel.firstName}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="last-name-input" type="text" defaultValue={this.componentModel.lastName}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row">{this.componentModel.userName}</div>

          <div className="personal-detail-row">
            <span title="Trader" className={traderClass}/>
            <span title="Manager" className={managerClass}/>
            <span title="Admin" className={adminClass}/>
            <span title="Service" className={serviceClass}/>
          </div>

          <div className="personal-detail-row">{registrationDate}</div>

          <div className="personal-detail-row">{this.componentModel.id}</div>

          <div className="personal-detail-row editable">
            <input className="email-input" type="text" defaultValue={this.componentModel.email}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <textarea defaultValue="" rows="1" className="address-input"/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="city-input" type="text" defaultValue={this.componentModel.city}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
            <input className="province-input" type="text" defaultValue={this.componentModel.province}
                   onChange={this.onChange.bind(this)}/>
            <div className="edit-icon-wrapper">
              <span className="icon-edit"/>
            </div>
          </div>

          <div className="personal-detail-row editable">
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
          <div className="labels-container">
            <div className="personal-label-row">First Name</div>
            <div className="personal-label-row">Last Name</div>
            <div className="personal-label-row">Username</div>
            <div className="personal-label-row">Role(s)</div>
            <div className="personal-label-row">Registration Date</div>
            <div className="personal-label-row">ID</div>
            <div className="personal-label-row">E-mail</div>
            <div className="personal-label-row address">Address</div>
            <div className="personal-label-row">City</div>
            <div className="personal-label-row">Province/State</div>
            <div className="personal-label-row">Country</div>
          </div>
          {details}
        </div>
    );
  }
}

export default View;