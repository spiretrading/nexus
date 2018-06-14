import {css, StyleSheet} from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

/** The React state for the TestApp. */
export interface State {
  rejectMessage: string;
};

/** The React properties for the TestAppåå. */
export interface Properties {

};

const model = new WebPortal.LocalLoginPageModel();
const onLogin = (account: Beam.DirectoryEntry) => {

};

/**  Displays a testing application for the login page. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      rejectMessage: ''
    };
  }

  public render(): JSX.Element {
    const onChange = (event: any) => {
      this.setState({rejectMessage: event.target.value});
    };
    return (
      <WebPortal.VBoxLayout width='100%' height='100%'>
        <WebPortal.LoginPage model={model} onLogin={onLogin}/>
        <div className={css(TestApp.STYLE.testingComponents)}>
        <button onClick={() => model.accept.bind(model)()}>
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
      position: 'absolute' as 'absolute',
      bottom: '100px',
      width: '160px',
      left: 'calc(50% - 80px)'
    }
  });
}
ReactDOM.render(<TestApp/>, document.getElementById('main'));
