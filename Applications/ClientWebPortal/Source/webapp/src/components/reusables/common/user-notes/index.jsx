import {Component} from 'react';
import Controller from './controller';
import View from './view';
import uuid from 'uuid';

class UserNotes extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    this.controller = new Controller(this);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentWillUpdate(nextProps) {
    this.controller.componentWillUpdate(nextProps.model);
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default UserNotes;