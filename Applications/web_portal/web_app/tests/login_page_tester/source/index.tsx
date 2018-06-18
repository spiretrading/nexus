import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const model = new WebPortal.LocalLoginPageModel();

/**  Displays a testing application for the login page. */
class TestApp extends React.Component<{}> {
  constructor(props: {}) {
    super(props);
    this.state = {
      rejectMessage: ''
    };
    this.onLogin = this.onLogin.bind(this);
  }

  public render(): JSX.Element {
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.LoginPage model={model} onLogin={this.onLogin}/>
        <div className={css(TestApp.STYLE.testingComponents)}>
        <button onClick={model.accept}>
          Accept
        </button>
        <button onClick={() => model.reject('Incorrect username or password')}>
          Reject: Invalid Username/ Password
        </button>
        <button onClick={() => model.reject('Server error')}>
          Reject - Server error
        </button>
        </div>
      </WebPortal.VBoxLayout>);
  }
  private static STYLE = StyleSheet.create({
    testingComponents: {
      position: 'fixed' as 'fixed',
      top: 0,
      left: 0,
      zIndex: 1
    }
  });

  private onLogin(account: Beam.DirectoryEntry) {
    console.log('Login with account: ', account)
  }
}
ReactDOM.render(<TestApp/>, document.getElementById('main'));
