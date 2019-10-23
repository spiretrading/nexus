import * as React from 'react';
import * as Nexus from 'nexus';
import { RuleExecutionDropDown } from './rule_execution_drop_down';
import { DisplaySize } from '../../../display_size';

interface Properties {
  displaySize: DisplaySize;
}

export class CompliancePage extends React.Component<Properties> {
  public render(): JSX.Element {
    return <RuleExecutionDropDown displaySize={this.props.displaySize}/>;
  }
}