import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const model = new WebPortal.LocalLoginPageModel();

ReactDOM.render(<WebPortal.LoginPage model={model}/>,
  document.getElementById('main'));
