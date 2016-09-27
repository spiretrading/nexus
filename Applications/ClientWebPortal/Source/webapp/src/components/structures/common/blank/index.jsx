import React, {Component} from 'react';

class Home extends Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    EventBus.publish(Event.Application.BLANK_PAGE_LOADED);
  }

  render() {
    return (
      <div id="blank-container">
      </div>
    );
  }
}

export default Home;
