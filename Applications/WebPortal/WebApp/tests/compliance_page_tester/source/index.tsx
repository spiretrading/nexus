import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

/** Displays a sample CompliancePage for testing. */
class TestApp extends React.Component<Properties> {
  public render(): JSX.Element {
    const ruleEntries  = [] as Nexus.ComplianceRuleEntry[];
    const someEntry = new Nexus.ComplianceRuleEntry(
      56,
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
    );
    ruleEntries.push(someEntry);
    const someEntry2 = new Nexus.ComplianceRuleEntry(
      34,
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
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(16000)))
      ]));
    ruleEntries.push(someEntry2);
    const ruleSchemas = [];
    ruleSchemas.push(
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
    ruleSchemas.push(
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
    ruleSchemas.push(
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
    ruleSchemas.push(
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
    ruleSchemas.push(
      new Nexus.ComplianceRuleSchema('Some Rule', [
        new Nexus.ComplianceParameter(
          'Currency',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(23424))),
        new Nexus.ComplianceParameter(
          'Money',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.CURRENCY, Nexus.DefaultCurrencies.CAD))
      ]));
    ruleSchemas.push(
      new Nexus.ComplianceRuleSchema('Timeout Period', [
        new Nexus.ComplianceParameter(
          'Time Out Duration',
          new Nexus.ComplianceValue(
            Nexus.ComplianceValue.Type.DURATION, new Beam.Duration(2342)))
      ]));
    ruleSchemas.push(
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
    return(
      <WebPortal.CompliancePage
        displaySize={this.props.displaySize} 
        ruleSchemas={ruleSchemas}
        currencyDatabase={Nexus.buildDefaultCurrencyDatabase()}
        complianceList={ruleEntries}/>);
  }
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage />, document.getElementById('main'));
