import {Component} from 'react';
import Controller from 'components/structures/common/risk-controls-modification-review/controller';
import View from './view';
import uuid from 'uuid';

class RiskControlsModificationReview extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.modificationId = this.props.location.state.id;
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

export default RiskControlsModificationReview;
