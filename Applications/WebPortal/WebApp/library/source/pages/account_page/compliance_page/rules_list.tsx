import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { RuleRow } from '.';

interface Properties {
  displaySize: DisplaySize;
  currencyDatabase: Nexus.CurrencyDatabase;
  complianceList: Nexus.ComplianceRuleEntry[];
  onChange?:(ruleIndex: number, newRule: Nexus.ComplianceRuleEntry) => void;
}

export class RulesList extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  public render(): JSX.Element {
    const rules = [];
    for (let i =0; i < this.props.complianceList.length; ++i) {
      const rule = this.props.complianceList[i]
      rules.push(
        <RuleRow
          displaySize={this.props.displaySize}
          complianceRule={rule}
          currencyDatabase={this.props.currencyDatabase}
          onChange={this.onChange.bind(this, i)}/>);
    }
    return (
      <div>
        {rules}
      </div>);
  }

  private onChange(ruleIndex: number, newRule: Nexus.ComplianceRuleEntry) {
    this.props.onChange(ruleIndex, newRule);
  }
}
