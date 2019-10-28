import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../../display_size';

interface Properties {
  
  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;
}

export class NewRuleButton extends React.Component<Properties>{
  constructor(props: Properties) {
    super(props);
  };

  public render(): JSX.Element {
    return (<div style={NewRuleButton.STYLE.newRuleRow}>
            <img src='resources/account_page/compliance_page/add.svg'
              height={'20px'}
              width={'20px'}/>
            <div style={NewRuleButton.STYLE.newRuleText}>Add New Rule</div>
    </div>);
  }

    private static readonly STYLE = {
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      font: '400 16px Roboto',
    },
    newRuleText: {
      font: '400 14px Roboto',
      height: '20px',
      paddingLeft: '18px'
    }
  }
}
