import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { RuleRow } from './rule_row';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The list of compliance rules. */
  complianceList: Nexus.ComplianceRuleEntry[];

  /** Indicates if the component is readonly. */
  readonly?: boolean;

  /** The event handler called when a rule entry changes. */
  onChange?: (updatedRule: Nexus.ComplianceRuleEntry) => void;
}

/** Displays a list of rules. */
export class RulesList extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  public render(): JSX.Element {
    const rules = [];
    for(let i = 0; i < this.props.complianceList.length; ++i) {
      const rule = this.props.complianceList[i];
      rules.push(
        <RuleRow
          displaySize={this.props.displaySize}
          complianceRule={rule}
          currencyDatabase={this.props.currencyDatabase}
          readonly={this.props.readonly}
          onChange={this.props.onChange}/>);
    }
    return (
      <div>
        {rules}
      </div>);
  }
}
