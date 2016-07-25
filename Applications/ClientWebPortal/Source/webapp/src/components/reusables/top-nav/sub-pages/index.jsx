import {Component} from 'react';
import Controller from './controller';
import './style.scss';

/** Sub pages that show up top right of the web site */
class SubPages extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentWillUpdate(nextProps, nextState) {
    this.controller.componentWillUpdate(nextProps, nextState);
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default SubPages;