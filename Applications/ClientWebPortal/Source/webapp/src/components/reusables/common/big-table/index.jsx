import {Component} from 'react';
import Controller from './controller';
import View from './view';

class BigTable extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    let componentModel = this.props || {};
    this.controller = new Controller(this, componentModel);
    this.view = new View(this, this.controller, componentModel);
    this.controller.setView(this.view);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount();
  }

  componentWillUpdate() {
    this.view.componentWillUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default BigTable;
