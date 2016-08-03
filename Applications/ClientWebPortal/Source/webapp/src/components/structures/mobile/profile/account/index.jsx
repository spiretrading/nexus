import {Component} from 'react';
import Controller from 'components/structures/common/profile/account/controller';
import View from './view';
import './style.scss';
import uuid from 'uuid';

class ProfileAccount extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
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

export default ProfileAccount;