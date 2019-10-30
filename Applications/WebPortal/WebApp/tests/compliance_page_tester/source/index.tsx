import * as Beam from 'beam';
import * as Nexus from 'nexus';
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
    const complianceList = [] as Nexus.ComplianceRuleEntry[];
    const sdir = Beam.DirectoryEntry.makeDirectory(34, 'Directory');
    const someEntry = new Nexus.ComplianceRuleEntry(
      124,
      sdir,
      Nexus.ComplianceRuleEntry.State.PASSIVE,
      new Nexus.ComplianceRuleSchema(
        'Some Rule',
        [new Nexus.ComplianceParameter('Money', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.MONEY, 100))]
      )
    );
    complianceList.push();
    return(
      <div style={TestApp.STYLE.wrapper}>
        <WebPortal.CompliancePage displaySize={this.props.displaySize}/>
      </div> );
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
