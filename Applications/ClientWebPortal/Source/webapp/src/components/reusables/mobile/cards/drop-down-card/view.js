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
      $(document).on('click' + this.componentModel.componentId, this.onDocumentClick.bind(this));
    }

    $('#' + this.componentModel.componentId + ' .select-container').click((event) => {
      event.preventDefault();
    });
  }

  dispose() {
    if (!this.componentModel.isReadOnly) {
      $(document).off('click' + this.componentModel.componentId);
    }

    $('#' + this.componentModel.componentId + ' .select-container').off('click');
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

    let clickEvent = new MouseEvent('mousedown', {
      view: window,
      bubbles: false,
      cancelable: false
    });
    $cardContainer.find('.select-container')[0].dispatchEvent(clickEvent);
  }

  onDeselected() {
    let $cardContainer = $('#' + this.componentModel.componentId);
    $cardContainer.removeClass('selected');
  }

  getOptions() {
    let options = [];
    options.push(
      <option key="0" value={'None'} disabled>None</option>
    );

    for (let i=0; i<this.componentModel.options.length; i++) {
      options.push(
        <option key={i+1} value={this.componentModel.options[i]}>{this.componentModel.options[i]}</option>
      );
    }

    return options;
  }

  onChange(event) {
    let currency = $('#' + this.componentModel.componentId).find('.select-container').val();
    this.controller.onSelectionChange.apply(this.controller, [currency]);
    this.selected = false;
    this.onDeselected();
  }

  render() {
    let currency;
    if (this.componentModel.isReadOnly) {
      currency = <div className="select-container">{this.componentModel.value}</div>
    } else {
      let options = this.getOptions.apply(this);
      currency =
        <select className="select-container" defaultValue={this.componentModel.value} onChange={this.onChange.bind(this)}>
          {options}
        </select>;
    }
    return (
      <div id={this.componentModel.componentId} className="card-container drop-down-card-container" onClick={this.onCardClick.bind(this)}>
        <div className="title">{this.componentModel.title}</div>
        <div className="body not-selectable">
          {currency}
        </div>
      </div>
    );
  }
}

export default View;
