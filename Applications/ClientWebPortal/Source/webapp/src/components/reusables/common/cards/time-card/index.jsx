import {Component} from 'react';
import Controller from './controller';
import View from './view';
import uuid from 'uuid';

class TimeCard extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.className = this.props.className;
    this.controller = new Controller(this, componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount.apply(this.controller);
  }

  componentWillUpdate(nextProps) {
    this.controller.componentWillUpdate(nextProps.model);
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount.apply(this.controller);
  }

  render() {
    return this.controller.getView().render();
  }
}

export default TimeCard;
