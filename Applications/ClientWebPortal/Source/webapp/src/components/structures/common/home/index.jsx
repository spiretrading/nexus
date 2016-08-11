import React, {Component} from 'react';
import Login from 'components/reusables/common/sign-in';
import './style.scss';

class Home extends Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    $('body').css('overflow', 'hidden');
    EventBus.publish(Event.Application.PAGE_LOADED);
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
