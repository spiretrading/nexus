import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';
import autosize from 'autosize';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.isLastInputLineBreak = false;
  }

  ignoreNewLineIfNecessary(event) {
    var numLineBreaks = (event.currentTarget.value.match(/\n/g) || []).length;
    if (event.keyCode === 13 && (numLineBreaks === 2 || this.isLastInputLineBreak || this.componentModel.addressLineOne.length === 0)) {
      event.preventDefault();
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
    let $componentContainer = $('#' + this.componentModel.componentId);
    this.componentModel.firstName = $componentContainer.find('.first-name-input').val().trim();
    this.componentModel.lastName = $componentContainer.find('.last-name-input').val().trim();
    this.componentModel.email = $componentContainer.find('.email-input').val().trim();
    this.componentModel.city = $componentContainer.find('.city-input').val().trim();
    this.componentModel.province = $componentContainer.find('.province-input').val().trim();
    this.componentModel.country = parseInt($componentContainer.find('.country-input').val().trim());
    this.componentModel.userName = $componentContainer.find('.user-name-input').val().trim();

    // address
    let addressLines = $componentContainer.find('.address-input').val().trim().split('\n');
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

    // roles
    this.componentModel.roles = {
      isTrader: $componentContainer.find('.icon-trader').hasClass('active'),
      isAdmin: $componentContainer.find('.icon-admin').hasClass('active'),
      isManager: $componentContainer.find('.icon-manager').hasClass('active'),
      isService: $componentContainer.find('.icon-service').hasClass('active')
    };

    // groups
    let selectedGroupNames = $componentContainer.find('.group-tags').tagit("assignedTags");
    let selectedGroupDirectoryEntries = [];
    for (let i=0; i<this.componentModel.managedTradingGroups.length; i++) {
      let groupDirectoryEntry = this.componentModel.managedTradingGroups[i];
      if (selectedGroupNames.indexOf(groupDirectoryEntry.name) >= 0) {
        selectedGroupDirectoryEntries.push(groupDirectoryEntry);
      }
    }
    this.componentModel.groups = selectedGroupDirectoryEntries;

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
  }

  /** @private */
  initializeGroupsTags() {
    let groupNames = [];
    for (let i=0; i<this.componentModel.managedTradingGroups.length; i++) {
      let groupDirectoryEntry = this.componentModel.managedTradingGroups[i];
      groupNames.push(groupDirectoryEntry.name);
    }

    $('#' + this.componentModel.componentId + ' .group-tags').tagit({
      availableTags: groupNames,
      autocomplete: {
        delay: 0,
        minLength: 1
      },
      tagLimit: 1,
      beforeTagAdded: (event, ui) => {
        if ($.inArray(ui.tagLabel, groupNames) == -1) {
          return false;
        }
      },
      afterTagAdded: () => {
        this.onChange();
      },
      afterTagRemoved: () => {
        this.onChange();
      }
    });
  }

  initialize() {
    this.initializeAddressInput();
    this.initializeGroupsTags();
  }

  dispose() {
    autosize.destroy($('#' + this.componentModel.componentId + ' .address-input'));
  }
}

export default CommonView;
