import {Component} from 'react';
import Controller from './controller';
import './style.scss';

/** Top nav react interface */
class Login extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  render() {
    return this.controller.getView().render();
  }
}

export default Login;