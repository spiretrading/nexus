import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import * as Beam from 'beam';

const model = new WebPortal.LocalLoginPageModel();
const onLogin = (account: Beam.DirectoryEntry) => {
	console.log(account)
}
ReactDOM.render(<WebPortal.LoginPage model={model} onLogin={onLogin}/>,
  document.getElementById('main'));
