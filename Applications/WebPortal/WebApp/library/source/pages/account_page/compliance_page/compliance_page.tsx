import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { RuleExecutionDropDown, RuleMode } from './rule_execution_drop_down';


interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

interface State {
  ruleMode: RuleMode;
}

/* Displays the compliance page.*/
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