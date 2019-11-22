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
      Nexus.ComplianceRuleEntry.State.ACTIVE,
      new Nexus.ComplianceRuleSchema(
        'Old Components',
        [
          new Nexus.ComplianceParameter(
            'Money',
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('1234.56'))),
          new Nexus.ComplianceParameter('Currency',
            new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.CURRENCY, 
              Nexus.DefaultCurrencies.CAD)),
          new Nexus.ComplianceParameter(
            'Note',
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.STRING, 'Keep an eye on this.')),
        ]
      )
    );
    list.push(someEntry);
    const someEntry2 = new Nexus.ComplianceRuleEntry(
      34,
      Beam.DirectoryEntry.makeDirectory(124, 'Directory'),
      Nexus.ComplianceRuleEntry.State.PASSIVE,
      new Nexus.ComplianceRuleSchema('New Components',[]));
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
      marginRight: '10px',
      boxSizing: 'border-box' as 'border-box',
    }
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage />, document.getElementById('main'));
