import React, {Component} from 'react';
import Login from 'components/reusables/common/sign-in';
import './style.scss';
import userService from 'services/user';

class Home extends Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    if (userService.isSignedIn()) {
      let directoryEntry = userService.getDirectoryEntry();
      let path = 'profile-account/' + directoryEntry.type + '/' + directoryEntry.id + '/' + directoryEntry.name;
      window.location.href = Config.HOME_URL + path;
    }

    $('body').css('overflow', 'hidden');
    $('main').css('padding', '0');
  }

  componentWillUnmount() {
    $('body').css('overflow', '');
    $('main').css('padding', '');
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
