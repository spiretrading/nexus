import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as Router from 'react-router-dom';
import { AccountEntry, RolePanel } from '../../..';

interface Properties {

  /** The account to display. */
  account: AccountEntry;
}

/** Displays the details of a member of the group. */
export class GroupMemberEntry extends React.Component<Properties> {

  public render(): JSX.Element {
    return (
      <Router.Link
          key={this.props.account.account.id} 
          to={`/account/${this.props.account.account.id}`}
          className={css(GroupMemberEntry.EXTRA_STYLE.wrapper)}>
        <span style={GroupMemberEntry.STYLE.name}>
          {this.props.account.account.name}
        </span>
        <span style={GroupMemberEntry.STYLE.roleWrapper}>
          <RolePanel roles={this.props.account.roles}/>
        </span>
      </Router.Link>
    ); 
  }

  private static readonly STYLE = {
    name: {
      color: '#333333',
      font: '400 14px Roboto'
    },
    roleWrapper: {
      width: '80px'
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
      border: 'none',
      textDecoration: 'none',
      ':hover': {
        backgroundColor: '#F8F8F8'
      },
      ':active': {
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none',
        backgroundColor: '#F8F8F8'
      },
      '::-moz-focus-inner': {
        border: 0,
        outline: 0
      },
      ':-moz-focusring': {
        outline: 0
      }
    }
  });
}
