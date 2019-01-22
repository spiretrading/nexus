import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

ReactDOM.render(<WebPortal.LoadingPage delay={2000}/>,
  document.getElementById('main'));
