import * as Beam from 'beam';
import { VBoxLayout, Padding } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { DisplaySize, DropDownButton } from '../../..';
import { AccountEntry } from '.';
import { RolePanel } from '../account_page/role_panel';
import { HLine } from '../../components';

interface Properties {

  /** Determines the size to display the component at. */
  displaySize: DisplaySize;

  /** The group the card belongs to. */
  group: Beam.DirectoryEntry;

  /** The accounts in the group. */
  accounts: AccountEntry[];

  /** The current filter used on the accounts. */
  filter: string;

  /**  Determines if the card is opened. */
  isOpen: boolean;

  /** Called when the card is clicked. */
  onClick: (group: Beam.DirectoryEntry) => void;
}

/** A card that displays a group and the accounts associated with it. */
export class GroupCard extends React.Component<Properties> {
  public render(): JSX.Element {
    const headerStyle = (() => {
      if(this.props.isOpen) {
        return GroupCard.STYLE.textOpen;
      } else {
        return GroupCard.STYLE.text;
      }
    })();
    const accounts = (() => {
      if(this.props.isOpen) {
        const stuff = [];
        stuff.push(<HLine color='#E6E6E6'/>);
        stuff.push(<Padding size='10px'/>);
        if(this.props.accounts.length > 0) {
          for(let i = 0; i < this.props.accounts.length; ++i) {
            stuff.push(
              <div style={GroupCard.STYLE.accountBox}
                  key={this.props.accounts[i].account.id}>
                <div style={GroupCard.STYLE.text}>
                  {this.props.accounts[i].account.name}
                </div>
                <RolePanel roles={this.props.accounts[i].roles}/>
              </div>);
          }
        } else {
          stuff.push(<div>Empty</div>);
        }
        stuff.push(<Padding size='20px'/>);
        return stuff;
      } else {
        return null;
      }
    })();
    return (
    <VBoxLayout width='100%'>
      <div style={GroupCard.STYLE.header}>
        <DropDownButton size='16px'
          onClick={(event?: React.MouseEvent<any>) => {
           this.props.onClick(this.props.group);}
          }/>
        <div style={headerStyle}>{this.props.group.name}</div>
      </div>
      {accounts}
    </VBoxLayout>);
  }

  private static readonly STYLE = {
    box: {
      width: '100%'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      marginLeft: '10px',
      marginRight: '10px'
    },
    headerSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      marginLeft: '10px',
      marginRight: '10px'
    },
    textOpen: {
      marginLeft: '18px',
      font: '500 14px Roboto',
      color: '#4B23A0'
    },
    text: {
      marginLeft: '18px',
      font: '400 14px Roboto',
      color: '#000000'
    },
    accountLabelSmall: {
      font: '400 14px Roboto',
      color: '#000000',
      marginRight: '10px'
    },
    accountLabelMedium: {
      font: '400 14px Roboto',
      color: '#000000',
      marginRight: '10px',
      marginLeft: '24px'
    },
    accountLabelLarge: {
      font: '400 14px Roboto',
      color: '#000000',
      marginRight: '10px',
      marginLeft: '38px'
    },
    accountBox: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      marginLeft: '10px',
      marginRight: '10px'
    }
  };
  private static readonly ANIMATION_STYLE = {
    entering: {

    },
    entered: {
   
    },
    exiting: {

    },
    exited: {

    }
  };
  private static readonly TRANSITION_LENGTH_MS = 600;
}
