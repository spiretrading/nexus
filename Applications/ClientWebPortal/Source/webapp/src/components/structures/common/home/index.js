import React, {Component} from 'react';
import Login from 'components/reusables/login';
import './style.scss';

class Home extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
        <div id="home-container">
          <div id="login-wrapper">
            <Login />
          </div>
        </div>
    );
  }
}

export default Home;
