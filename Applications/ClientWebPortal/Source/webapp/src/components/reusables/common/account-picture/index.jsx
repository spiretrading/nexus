import {Component} from 'react';
import Controller from './controller';

class AccountPicture extends Component {
  constructor(props) {
    super(props);
  }

  componentWillMount() {
    this.controller = new Controller(this);
  }

  componentWillUpdate(nextProps) {
    this.controller.componentWillUpdate(nextProps.model);
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

export default AccountPicture;
