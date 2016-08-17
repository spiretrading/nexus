import {Component} from 'react';
import Controller from './controller';

/** Signin form */
class SignIn extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentDidMount() {
    this.controller.componentDidMount();
  }

  componentDidUpdate() {
    this.controller.getView().componentDidUpdate();
  }

  componentWillUnmount() {
    this.controller.componentWillUnmount();
  }

  render() {
    return this.controller.getView().render();
  }
}

export default SignIn;