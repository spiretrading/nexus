import {Component} from 'react';
import Controller from 'components/structures/common/profile/risk-controls/request-confirmation/controller';
import View from './view';
import uuid from 'uuid';

class ProfileRiskControls extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.modificationId = parseInt(this.props.routeParams.id);
    this.controller = new Controller(componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  render() {
    return this.view.render();
  }
}

export default ProfileRiskControls;
