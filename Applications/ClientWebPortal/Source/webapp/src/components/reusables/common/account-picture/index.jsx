import {Component} from 'react';
import Controller from './controller';
import './style.scss';

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

  render() {
    return this.controller.getView().render();
  }
}

export default AccountPicture;