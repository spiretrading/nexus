import {Component} from 'react';
import Controller from './controller';

/** Sub pages that show up top right of the web site */
class SubPages extends Component {
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

export default SubPages;
