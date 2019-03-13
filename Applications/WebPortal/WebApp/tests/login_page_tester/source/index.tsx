import * as Beam from 'beam';
import * as Dali from 'dali';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

/**  Displays a testing application for the login page. */
class TestApp extends React.Component {
  constructor(props: {}) {
    super(props);
    this.model = new WebPortal.LocalLoginModel();
    this.onLogin = this.onLogin.bind(this);
  }

  public render(): JSX.Element {
    return (
      <Dali.VBoxLayout width='100%' height='100%'>
        <WebPortal.LoginController model={this.model} onLogin={this.onLogin}/>
        <div style={TestApp.STYLE}>
          <button tabIndex={-1} onClick={() => this.model.accept()}>
            Accept
          </button>
          <button tabIndex={-1} onClick={
              () => this.model.reject('Incorrect username or password')}>
            Reject: Invalid Username/ Password
          </button>
          <button tabIndex={-1}
              onClick={() => this.model.reject('Server error')}>
            Reject - Server error
          </button>
        </div>
      </Dali.VBoxLayout>);
  }

  private onLogin(account: Beam.DirectoryEntry) {
    console.log('Login with account: ', account)
  }

  private static readonly STYLE = {
    position: 'fixed' as 'fixed',
    top: 0,
    left: 0,
    zIndex: 1
  };

  model: WebPortal.LocalLoginModel;
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));
