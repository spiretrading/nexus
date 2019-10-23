import * as React from 'react';
import * as Nexus from 'nexus';
import { RuleExecutionDropDown, RuleMode } from './rule_execution_drop_down';
import { DisplaySize } from '../../../display_size';

interface Properties {
  displaySize: DisplaySize;
}

interface State {
  ruleMode: RuleMode;
}

export class CompliancePage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      ruleMode: RuleMode.PASSIVE
    };
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    return <RuleExecutionDropDown 
      displaySize={this.props.displaySize}
      value={this.state.ruleMode}
      onChange={this.onChange}/>;
  }

  private onChange(newMode: RuleMode) {
    this.setState({ruleMode: newMode});
  }
}