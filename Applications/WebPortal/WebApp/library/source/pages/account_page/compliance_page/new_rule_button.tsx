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
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    return (
      <div style={NewRuleButton.STYLE.newRuleRow}>
            <div style={NewRuleButton.STYLE.imageWrapper}>
              <img src='resources/account_page/compliance_page/add.svg'
                height={imageSize}
                width={imageSize}/>
            </div>
            <div style={NewRuleButton.STYLE.newRuleText}>Add New Rule</div>
    </div>);
  }

    private static readonly STYLE = {
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center'
    },
    newRuleText: {
      font: '400 14px Roboto',
      paddingLeft: '18px'
    },
    imageWrapper: {
      position: 'relative' as 'relative',
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      width: '20px',
      height: '20px',
      cursor: 'pointer'
    }
  }
}
