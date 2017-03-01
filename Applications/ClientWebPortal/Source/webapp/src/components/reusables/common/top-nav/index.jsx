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
    this.controller.componentDidMount.apply(this.controller);
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount.apply(this.controller);
  }

  render() {
    return this.controller.getView().render();
  }
}

export default TopNav;
