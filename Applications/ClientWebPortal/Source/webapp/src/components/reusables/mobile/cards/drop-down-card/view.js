import 'components/reusables/common/cards/style.scss';
import './style.scss';
import React from 'react';
import UpdatableView from 'commons/updatable-view';

class View extends UpdatableView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
    this.selected = false;
  }

  initialize() {
    if (!this.componentModel.isReadOnly) {
      $(document).on('click.' + this.componentModel.componentId, this.onDocumentClick.bind(this));
    }
  }

  dispose() {
    if (!this.componentModel.isReadOnly) {
      $(document).off('click.' + this.componentModel.componentId);
    }
  }

  onDocumentClick(event) {
    let cardContainerElement = $('#' + this.componentModel.componentId)[0];
    let clickedElement = event.target;
    let isCardClicked = $.contains(cardContainerElement, clickedElement) || cardContainerElement === clickedElement;

    if (!isCardClicked && this.selected) {
      this.selected = false;
      this.onDeselected();
    }
  }

  onCardClick() {
    console.debug('card is clicked');
    console.debug(this.selected);
    if (!this.componentModel.isReadOnly) {
      if (this.selected) {
        this.selected = false;
        this.onDeselected();
      } else {
        this.selected = true;
        this.onSelected();
      }
    }
  }

  onSelected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.addClass('selected');
    $cardContainer.find('.select-container').simulate('mousedown');
  }

  onDeselected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
  }

  getOptions() {
    let options = [];
    for (let i=0; i<this.componentModel.options.length; i++) {
      options.push(
        <option key={i} value={this.componentModel.options[i]}>{this.componentModel.options[i]}</option>
      );
    }
    return options;
  }

  onChange(event) {
    let currency = $('#' + this.componentModel.componentId).find('.select-container').val();
    this.controller.onChange(currency);
    this.selected = false;
    this.onDeselected();
  }

  render() {
    let options = this.getOptions.apply(this);
    return (
      <div id={this.componentModel.componentId} className="card-container drop-down-card-container" onClick={this.onCardClick.bind(this)}>
        <div className="title">{this.componentModel.title}</div>
        <div className="body not-selectable">
          <select className="select-container" defaultValue={this.componentModel.value} onChange={this.onChange.bind(this)}>
            {options}
          </select>
        </div>
      </div>
    );
  }
}

export default View;
