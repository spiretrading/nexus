import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { AccountEntry, RolePanel } from '../../..';

interface Properties {

  /** The account to display. */
  account: AccountEntry;
}

/** Displays the details of the members of a group. */
export class GroupMemberEntry extends React.Component<Properties> {

  public render(): JSX.Element {
    const {account} = this.props;
    return (
      <span className={css(GroupMemberEntry.EXTRA_STYLE.wrapper)}>
        <div style={GroupMemberEntry.STYLE.name}>{account.account.name}</div>
        <RolePanel roles={account.roles}/>
      </span>
    ); 
  }

  private static readonly STYLE = {
    name: {
      color: '#333333',
      font: '400 14px Roboto',
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    wrapper: {
      boxSizing: 'border-box',
      height: '34px',
      display: 'flex',
      width: '100%',
      justifyContent: 'space-between',
      paddingLeft: '10px',
      paddingRight: '10px',
      alignItems: 'center',
      backgroundColor: '#FFFFFF',
      ':hover': {
        backgroundColor: '#F8F8F8'
      },
      ':active': {
        backgroundColor: '#F8F8F8'
      }
    }
  });
}
