import React, {PropTypes} from 'react';
import TopNav from 'components/reusables/top-nav';
import SideMenu from 'components/reusables/side-menu';
import './style.scss';

const App = (props) => (
  <main className="viewport">
    <div id="top-nav-wrapper">
      <TopNav />
    </div>
    <div id="top-nav-filler"></div>
    <div id="side-menu-wrapper">
      <SideMenu />
    </div>
    {props.children}
  </main>
);

App.propTypes = {
  children: PropTypes.node
};

export default App;
