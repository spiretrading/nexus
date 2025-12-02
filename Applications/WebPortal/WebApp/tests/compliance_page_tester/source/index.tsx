import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  model: WebPortal.ComplianceModel;
  readonly: boolean;
}

function makeSchemas() {
  const schemas = [];
  schemas.push(
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
  schemas.push(
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
  schemas.push(
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
  schemas.push(
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
  schemas.push(
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
  schemas.push(
    new Nexus.ComplianceRuleSchema('Timeout Period', [
      new Nexus.ComplianceParameter(
        'Time Out',
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(2342)))
    ]));
  schemas.push(
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
  return schemas;
}

function makeEntries() {
  const entries = [];
  entries.push(new Nexus.ComplianceRuleEntry(
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
  entries.push(new Nexus.ComplianceRuleEntry(
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
              Nexus.Security.parse('S32.ASX')),
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.SECURITY,
              Nexus.Security.parse('TD.TSX')),
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.SECURITY,
              Nexus.Security.parse('RY.TSX')),
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.SECURITY,
              Nexus.Security.parse('SHOP.TSX')),
            new Nexus.ComplianceValue(
              Nexus.ComplianceValue.Type.SECURITY,
              Nexus.Security.parse('RIO.ASX'))
          ])),
      new Nexus.ComplianceParameter(
        'Security',
        new Nexus.ComplianceValue(
          Nexus.ComplianceValue.Type.SECURITY,
          Nexus.Security.parse('TD.TSX')))
    ])));
  return entries;
}

/** Displays a sample CompliancePage for testing. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    const account = Beam.DirectoryEntry.makeAccount(2201, 'trader');
    const model = new WebPortal.ComplianceModel(account, makeSchemas(),
      makeEntries(), Nexus.buildDefaultCurrencyDatabase());
    this.state = {
      model,
      readonly: false
    };
  }

  public render(): JSX.Element {
    const role = this.state.readonly &&
      Nexus.AccountRoles.Role.TRADER || Nexus.AccountRoles.Role.ADMINISTRATOR;
    return (
      <div style={{backgroundColor: 'black', height: '100%'}}>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1} onClick={this.toggleReadonly}>
            TOGGLE READONLY
          </button>
        </div>
        <WebPortal.PageWrapper>
          <WebPortal.CompliancePage displaySize={this.props.displaySize}
            roles={new Nexus.AccountRoles(role)} model={this.state.model}
            onRuleAdd={this.onRuleAdd} onRuleChange={this.onRuleChange}/>
        </WebPortal.PageWrapper>
      </div>);
  }

  private onRuleChange = (updatedRule: Nexus.ComplianceRuleEntry) => {
    this.setState(state => {
      state.model.update(updatedRule);
      return {model: state.model};
    });
  }

  private onRuleAdd = (newSchema: Nexus.ComplianceRuleSchema) => {
    this.setState(state => {
      state.model.add(newSchema);
      return {model: state.model};
    });
  }

  private toggleReadonly = () => {
    this.setState(state => { readonly: !state.readonly });
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
