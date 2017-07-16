import React from 'react';
import modal from 'utils/modal';
import UpdatableView from 'commons/updatable-view';

class CommonView extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);

    this.dispose = this.dispose.bind(this);
  }

  componentDidUpdate() {
    $('#search-profiles-container').fadeIn({
      duration: Config.FADE_DURATION
    });

    $('#search-profiles-container .search-wrapper .search-input').val(this.componentModel.searchString);
  }

  onNewAccountClick() {
    this.controller.navigateToNewAccount();
  }

  onNewGroupClick() {
    modal.show($('#new-group-modal'), () => {
      $('#new-group-modal .name-input').focus();
    });
  }

  closeModal() {
    modal.hide($('#new-group-modal'));
  }

  onSearchInputChange(event) {
    let searchString = $(event.currentTarget).val();
    if (this.searchInputTimeout != null) {
      clearTimeout(this.searchInputTimeout);
    }

    this.searchInputTimeout = setTimeout(() => {
      this.controller.search(searchString);
    }, Config.INPUT_TIMEOUT_DURATION);
  }

  onNewGroupCancelClick() {
    $('#new-group-modal .name-input').val('');
    this.closeModal();
  }

  onNewGroupCreateClick() {
    let $groupNameInput = $('#new-group-modal .name-input');
    let groupName = $groupNameInput.val().trim();
    this.controller.createGroup(groupName);
  }

  onGroupNamePress(event) {
    if (event.key === 'Enter') {
      let $groupNameInput = $('#new-group-modal .name-input');
      let groupName = $groupNameInput.val().trim();
      this.controller.createGroup(groupName);
    }
  }

  closeCreateGroupModal() {
    let $groupNameInput = $('#new-group-modal .name-input');
    $groupNameInput.val('');
    this.closeModal();
  }

  dispose() {
    this.closeModal();
  }
}

export default CommonView;
