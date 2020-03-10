import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  entries: Nexus.ComplianceRuleEntry[];
  schemas: Nexus.ComplianceRuleSchema[];
}

/** Displays a sample CompliancePage for testing. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      entries: [],
      schemas: []
    };
    this.onRuleAdd = this.onRuleAdd.bind(this);
    this.onRuleChange = this.onRuleChange.bind(this);
  }

  public render(): JSX.Element {
    return(
      <div style={{height: '100%'}}>
        <WebPortal.PageWrapper>
          <WebPortal.CompliancePage
            onRuleAdd={this.onRuleAdd}
            onRuleChange={this.onRuleChange}
            displaySize={this.props.displaySize}
            schemas={this.state.schemas}
            currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
            entries={this.state.entries}/>
        </WebPortal.PageWrapper>
      </div>);
  }

  public componentDidMount() {
    this.generateSchemas();
  }

  private generateSchemas() {
    this.state.entries.push(new Nexus.ComplianceRuleEntry(
      52,
      Beam.DirectoryEntry.makeDirectory(124, 'Directory'),
      Nexus.ComplianceRuleEntry.State.ACTIVE,
      new Nexus.ComplianceRuleSchema(
        'Old Components', [
          new Nexus.ComplianceParameter(
            'Money',
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('1234.56'))),
          new Nexus.ComplianceParameter(
            'Currency',
            new Nexus.ComplianceValue(Nexus.ComplianceValue.Type.CURRENCY,
              Nexus.DefaultCurrencies.CAD)),
          new Nexus.ComplianceParameter(
            'Note',
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.STRING, 'Keep an eye on this.'))
        ]
      )
    ));
    this.state.entries.push(new Nexus.ComplianceRuleEntry(
      88,
      Beam.DirectoryEntry.makeDirectory(124, 'Directory'),
      Nexus.ComplianceRuleEntry.State.PASSIVE,
      new Nexus.ComplianceRuleSchema('New Components', [
        new Nexus.ComplianceParameter(
          'Decimal',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DOUBLE, 345.686868)),
        new Nexus.ComplianceParameter(
          'Quantity',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.QUANTITY, 234.56)),
        new Nexus.ComplianceParameter(
          'Duration',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(16000))),
        new Nexus.ComplianceParameter(
          'Boolean',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.BOOLEAN, true)),
        new Nexus.ComplianceParameter(
          'Date/Time',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DATE_TIME,
            new Beam.DateTime(new Beam.Date(2004, 2, 4),
              new Beam.Duration(5105000)))),
        new Nexus.ComplianceParameter(
          'Securities',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.LIST, [
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('AST.XASX',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('PST.CSE',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('NQR.YYYY',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA)),
              new Nexus.ComplianceValue(
                Nexus.ComplianceValue.Type.SECURITY,
                new Nexus.Security('AST.NYC',
                  Nexus.MarketCode.NONE, Nexus.DefaultCountries.CA))
            ])),
        new Nexus.ComplianceParameter(
          'Security',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.SECURITY,
            new Nexus.Security('NQR.YYYY', Nexus.MarketCode.NONE,
              Nexus.DefaultCountries.CA)))
      ])));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Buying Power', [
        new Nexus.ComplianceParameter(
          'Currency',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000'))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Max Payout', [
        new Nexus.ComplianceParameter(
          'Currency',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000'))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Payout Range', [
        new Nexus.ComplianceParameter(
          'Min',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000'))),
        new Nexus.ComplianceParameter(
          'Max',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000')))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Min Payout', [
        new Nexus.ComplianceParameter(
          'Currency',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000'))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Some Rule', [
        new Nexus.ComplianceParameter(
          'Time',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(23424))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Timeout Period', [
        new Nexus.ComplianceParameter(
          'Time Out',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(2342)))
      ]));
    this.state.schemas.push(
      new Nexus.ComplianceRuleSchema('Rule with many Parameters', [
        new Nexus.ComplianceParameter(
          'Currency',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.MONEY, Nexus.Money.parse('10000'))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD)),
        new Nexus.ComplianceParameter(
          'Some Number',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.QUANTITY, 2423)),
        new Nexus.ComplianceParameter(
          'Some Double',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DOUBLE, 123.4567)),
      ]));
    this.setState({
      entries: this.state.entries,
      schemas: this.state.schemas
    });
  }

  private onRuleChange(updatedRule: Nexus.ComplianceRuleEntry) {
    for(let i = 0; i< this.state.entries.length; ++i) {
      if(this.state.entries[i].id === updatedRule.id) {
        this.state.entries[i] = updatedRule;
        this.setState({entries: this.state.entries});
      }
    }
  }

  private onRuleAdd(newSchema: Nexus.ComplianceRuleSchema) {
      this.state.entries.push(
      new Nexus.ComplianceRuleEntry(
        this.state.entries[this.state.entries.length - 1].id + 1,
        Beam.DirectoryEntry.INVALID,
        Nexus.ComplianceRuleEntry.State.DISABLED,
        new Nexus.ComplianceRuleSchema(
          newSchema.name, newSchema.parameters.slice())
      )
    );
    this.setState({entries: this.state.entries});
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      fontSize: '8px',
      top: 0,
      left: 0,
      zIndex: 500
    }
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
