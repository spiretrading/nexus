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
    $cardContainer.find('.options-wrapper').css('display', 'inherit');
  }

  onDeselected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
    $cardContainer.find('.options-wrapper').css('display', 'none');
  }

  getOptions() {
    let options = [];
    for (let i=0; i<this.componentModel.options.length; i++) {
      options.push(
        <div key={i} className="option" data-value={this.componentModel.options[i]} onClick={onOptionClicked.bind(this)}>
          {this.componentModel.options[i]}
        </div>
      );
    }
    return options;

    function onOptionClicked(event) {
      let clickedValue = $(event.currentTarget).attr('data-value');
      this.controller.onChange(clickedValue);
    }
  }

  render() {
    let options = this.getOptions.apply(this);
    return (
      <div id={this.componentModel.componentId} className="card-container" onClick={this.onCardClick.bind(this)}>
        <div className="title">{this.componentModel.title}</div>
        <div className="body not-selectable">{this.componentModel.value}</div>
        <div className="options-wrapper">
          <div className="options-container">
            {options}
          </div>
        </div>
      </div>
    );
  }
}

export default View;
