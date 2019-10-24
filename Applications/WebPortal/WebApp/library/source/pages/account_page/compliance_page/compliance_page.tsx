import * as React from 'react';
import { DisplaySize } from '../../../display_size';
import { RuleExecutionDropDown, RuleMode } from './rule_execution_drop_down';
import { RuleRow } from './rule_row';


interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

/* Displays the compliance page.*/
export class CompliancePage extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      ruleMode: RuleMode.PASSIVE
    };
    
  }

  public render(): JSX.Element {
    return( 
      <RuleRow displaySize={this.props.displaySize}/>);
  }


}