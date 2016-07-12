import {Component} from 'react';
import Controller from './controller';
import './style.scss';

/** Login form react interface */
class Login extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default Login;