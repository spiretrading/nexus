import {Component} from 'react';
import Controller from 'components/structures/common/profile/compliance/controller';
import View from './view';
import uuid from 'uuid';

class ProfileAccount extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.directoryEntry = {
      id: parseInt(this.props.routeParams.id),
      name: this.props.routeParams.name,
      type: parseInt(this.props.routeParams.type)
    };
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
    return this.view.render.apply(this.view);
  }
}

export default ProfileAccount;
