import {Component} from 'react';
import Controller from './controller';

/** Top navigational menu */
class TopNav extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default TopNav;