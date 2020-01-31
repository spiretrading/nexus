import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  entries: Nexus.ComplianceValue[];
  readonly: boolean;
}

/** Displays a sample CompliancePage for testing. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      entries: [],
      readonly: false
    };
  }

  public render(): JSX.Element {
    return(
      <WebPortal.PageWrapper>
        <button onClick={this.onClick}>Toggle Readonly</button>
        <WebPortal.SecurityInput
          value={this.state.entries}
          displaySize={this.props.displaySize}
          readonly={this.state.readonly}
          />
      </WebPortal.PageWrapper>);
  }

  public componentDidMount() {
    this.state.entries.push(
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.SECURITY,
          new Nexus.Security('AST.XASX', 
            Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)));
    this.state.entries.push(
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.SECURITY,
          new Nexus.Security('PST.CSE',
            Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)));
    this.state.entries.push(
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.SECURITY,
          new Nexus.Security('WER.CSE',
            Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)));
    this.state.entries.push(
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.SECURITY,
          new Nexus.Security('ASK.CSE',
            Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)));
      this.setState({
        entries: this.state.entries
      });
  }

  private onClick() {
    this.setState({readonly: !this.state.readonly});
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
