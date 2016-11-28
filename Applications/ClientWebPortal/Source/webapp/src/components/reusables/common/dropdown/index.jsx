import {Component} from 'react';
import Controller from './controller';
import View from './view';
import uuid from 'uuid';

class Dropdown extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props.model || {};
    componentModel.componentId = uuid.v4();
    componentModel.className = this.props.className;
    this.controller = new Controller(this, componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView.apply(this.controller, [this.view]);
  }

  componentWillUpdate(nextProps) {
    this.controller.componentWillUpdate(nextProps.model);
  }

  componentDidUpdate() {
    this.controller.componentDidUpdate.apply(this.controller);
  }

  render() {
    return this.controller.getView().render();
  }
}

export default Dropdown;
