import {Component} from 'react';
import Controller from './controller';
import './style.scss';

/** Navigation bar with basic user info displayed */
class UserInfoNav extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentWillUpdate(nextProps) {
    this.controller.componentWillUpdate(nextProps);
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default UserInfoNav;