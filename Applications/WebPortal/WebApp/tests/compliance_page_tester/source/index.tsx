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
    const list  = [] as Nexus.ComplianceRuleEntry[];
    const someEntry = new Nexus.ComplianceRuleEntry(
      56,
      Beam.DirectoryEntry.makeDirectory(124, 'Directory'),
      Nexus.ComplianceRuleEntry.State.PASSIVE,
      new Nexus.ComplianceRuleSchema(
        'Old Components',
        [
        new Nexus.ComplianceParameter('Money', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.MONEY, 100)),
        new Nexus.ComplianceParameter('Currency', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD)),
        new Nexus.ComplianceParameter('Note', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.STRING, 50000)),
        ]
      )
    );
    list.push(someEntry);
    const someEntry2 = new Nexus.ComplianceRuleEntry(
      34,
      Beam.DirectoryEntry.makeDirectory(124, 'Directory'),
      Nexus.ComplianceRuleEntry.State.PASSIVE,
      new Nexus.ComplianceRuleSchema(
        'New Components',
        [
        new Nexus.ComplianceParameter('Quantity', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.QUANTITY, 30)),
        new Nexus.ComplianceParameter('Double', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.DOUBLE, 12.34)),
        new Nexus.ComplianceParameter('Duration', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.DURATION, null)),
        new Nexus.ComplianceParameter('Date and Time', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.DATE_TIME, null)),
        new Nexus.ComplianceParameter('Boolean', new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.BOOLEAN, true))
        ]
      )
    );
    list.push(someEntry2);
    return(
      <div style={TestApp.STYLE.wrapper}>
        <WebPortal.CompliancePage 
          displaySize={this.props.displaySize} 
          currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
          complianceList={list}/>
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
