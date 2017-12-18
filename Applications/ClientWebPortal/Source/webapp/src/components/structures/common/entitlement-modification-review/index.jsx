import {Component} from 'react';
import Controller from './controller';
import View from './view';
import uuid from 'uuid';

export default class extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.modificationId = this.props.location.state.id;
    componentModel.requesterAccount = this.props.location.state.requesterAccount;
    componentModel.changeAccount = this.props.location.state.changeAccount;
    componentModel.requestStatus = this.props.location.state.requestStatus;
    this.controller = new Controller(componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.view.render();
  }
}
