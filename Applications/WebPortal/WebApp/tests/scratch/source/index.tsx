import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';
import { Padding } from 'web_portal';

class TestApp extends React.Component<{}, {}> {

  public render(): JSX.Element {
    return (
      <WebPortal.HBoxLayout style = {TestApp.STYLE.horizonal}>
        <WebPortal.VBoxLayout style = {TestApp.STYLE.vert}>
          <WebPortal.VBoxLayout>
            <WebPortal.HBoxLayout>
              <div>STUFF</div><Padding size={'100px'}/>
            </WebPortal.HBoxLayout>
          </WebPortal.VBoxLayout>
          
          </WebPortal.VBoxLayout>
      </WebPortal.HBoxLayout>
    );
  }

  private static STYLE = {
    horizonal: {
      backgroundColor: '#00496d'
    },
    vert: {
      backgroundColor: '#00496d'
    },
    padding: {
      backgroundColor: '#42bc58'
    }
  };
}





ReactDOM.render(<div />, document.getElementById('main'));
