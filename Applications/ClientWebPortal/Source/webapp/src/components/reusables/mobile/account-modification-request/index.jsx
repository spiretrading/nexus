import {Component} from 'react';
import Controller from 'components/reusables/common/account-modification-request/controller';
import View from './view';
import uuid from 'uuid';

export default class extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = {};
    componentModel.componentId = uuid.v4();
    componentModel.className = this.props.className;
    componentModel.request = this.props.request;
    componentModel.update = this.props.update;
    this.controller = new Controller(this, componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  render() {
    return this.controller.getView().render();
  }
}
