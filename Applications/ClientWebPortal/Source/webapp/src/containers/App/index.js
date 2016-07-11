import React, {PropTypes} from 'react';
import TopNav from 'components/reusables/top-nav';
import './style.scss';

const App = (props) => (
    <main className="viewport">
        <TopNav />
        {props.children}
    </main>
);

App.propTypes = {
    children: PropTypes.node
};

export default App;
