import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

/**  Displays a testing application. */
class TestApp extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <WebPortal.AccountDirectoryPage
        onNewAccountClick={null}
        onNewGroupClick={null}
        displaySize={this.props.displaySize}
        model={null}/>
    );
  }
}

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
