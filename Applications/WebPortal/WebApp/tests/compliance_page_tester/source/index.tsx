import * as Dali from 'dali';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

class TestApp extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    return(
    <div style={TestApp.STYLE.wrapper}>
      <WebPortal.CompliancePage displaySize={this.props.displaySize}/>
    </div> 
    );
  }

  private static readonly STYLE = {
    wrapper: {
      marginTop: '200px',
      marginBottom: '200px',
      marginLeft: '10px',
      marginRight: '10px'
    }
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage />, document.getElementById('main'));