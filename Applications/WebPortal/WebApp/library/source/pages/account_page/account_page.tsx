import * as Beam from 'beam';
import { HBoxLayout, Padding, VBoxLayout } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine, RolePanel } from '../..';
import { MenuBar } from './menu_bar';
import { SubPage } from './sub_page';

interface Properties {

  /** The account being displayed. */
  account: Beam.DirectoryEntry;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The sub page currently selected. */
  subPage: SubPage;

  /** Determines the layout used to render the page. */
  displaySize: DisplaySize;

  /** Indicates a sub-page menu item was clicked.
   * @param subPage - The SubPage that was clicked.
   */
  onMenuClick?: (subPage: SubPage) => void;
}

/** Implements the container used to display account information. */
export class AccountPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    onMenuClick: () => {}
  }

  public render(): JSX.Element {
    const Header = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.LARGE:
          return LargeHeader;
        case DisplaySize.MEDIUM:
          return MediumHeader;
        case DisplaySize.SMALL:
          return SmallHeader;
      }
    })();
    return (
      <VBoxLayout height='100%' width='100%'
          style={{overflow: 'hidden' as 'hidden'}}>
        <Header name={this.props.account.name} roles={this.props.roles}>
          <MenuBar displaySize={this.props.displaySize}
            selected={this.props.subPage} onClick={this.props.onMenuClick}/>
        </Header>
        {this.props.children}
      </VBoxLayout>);
  }
}

class HeaderUnderline extends React.PureComponent {
  public render(): JSX.Element {
    return <HLine color='#E6E6E6' height='1px'/>;
  }
}

interface HeaderProps {
  name: string;
  roles: Nexus.AccountRoles;
}

class LargeHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <Padding/>
          <HBoxLayout width='1036px' height='40px'>
            <Padding size='18px'/>
            {this.props.children}
            <Padding/>
            <div style={LargeHeader.STYLES.usernameAndRoleContainer}>
              <div style={LargeHeader.STYLES.usernameAndRoleWrapper}>
                <UsernameLabel name={this.props.name} height='40px'/>
                <Padding size='10px'/>
                <div style={LargeHeader.STYLES.roleWrapper}>
                  <RolePanel roles={this.props.roles}/>
                </div>
              </div>
            </div>
            <Padding size='18px'/>
          </HBoxLayout>
          <Padding/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static readonly STYLES = {
    usernameAndRoleWrapper: {
      height: '40px',
      width: 'auto',
      display: 'flex'
    },
    usernameAndRoleContainer: {
      height: '40px',
      display: 'flex',
      flexWrap: 'wrap' as 'wrap'
    },
    roleWrapper: {
      width: '68px',
      height: '40px',
      flexGrow: 0,
      flexShrink: 0
    }
  }
}

class MediumHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <div style={MediumHeader.STYLES.headerPadding}/>
          <Padding size='18px'/>
          <HBoxLayout height='40px' width='750px'>
            {this.props.children}
            <div style={MediumHeader.STYLES.innerPadding}/>
            <div style={MediumHeader.STYLES.roleWrapper}>
              <RolePanel roles={this.props.roles}/>
            </div>
          </HBoxLayout>
          <Padding size='18px'/>
          <div style={MediumHeader.STYLES.headerPadding}/>
        </HBoxLayout>
        <HeaderUnderline/>
        <HBoxLayout width='100%' height='30px'>
          <div style={MediumHeader.STYLES.headerPadding}/>
          <Padding size='18px'/>
          <HBoxLayout height='30px' width='750px'>
            <Padding/>
            <UsernameLabel name={this.props.name} height='30px'/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div style={MediumHeader.STYLES.headerPadding}/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static readonly STYLES = {
    headerPadding: {
      width: 'calc(50% - 393px)',
      height: '100%'
    },
    innerPadding: {
      width: 'calc(100% - 658px)',
      height: '100%'
    },
    roleWrapper: {
      width: '68px',
      height: '40px',
      flexGrow: 0,
      flexShrink: 0
    }
  }
}

class SmallHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <div style={SmallHeader.STYLES.headerPadding}/>
          <Padding size='18px'/>
          <HBoxLayout height='40px' style={SmallHeader.STYLES.accountHeader}>
            {this.props.children}
            <div style={SmallHeader.STYLES.innerPadding}/>
            <div style={SmallHeader.STYLES.roleWrapper}>
              <RolePanel roles={this.props.roles}/>
            </div>
          </HBoxLayout>
          <Padding size='18px'/>
          <div style={SmallHeader.STYLES.headerPadding}/>
        </HBoxLayout>
        <HeaderUnderline/>
        <HBoxLayout width='100%' height='30px'>
          <div style={SmallHeader.STYLES.headerPadding}/>
          <Padding size='18px'/>
          <HBoxLayout height='30px' style={SmallHeader.STYLES.accountHeader}>
            <Padding/>
            <UsernameLabel name={this.props.name} height='30px'/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div style={SmallHeader.STYLES.headerPadding}/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static readonly STYLES = {
    accountHeader: {
      width: '60%',
      minWidth: '284px',
      maxWidth: '424px'
    },
    headerPadding: {
      width: 'calc(20% - 18px)'
    },
    innerPadding: {
      width: 'calc(45% - 68px)',
      height: '100%'
    },
    roleWrapper: {
      width: '68px',
      height: '40px',
      flexGrow: 0,
      flexShrink: 0
    }
  }
}

interface UsernameProps {
  name: string;
  height: string;
}

class UsernameLabel extends React.Component<UsernameProps>  {
  public render(): JSX.Element {
    return (
      <VBoxLayout height={this.props.height}>
        <Padding/>
        <span style={UsernameLabel.STYLES.username}>
          {this.props.name}
        </span>
        <Padding/>
      </VBoxLayout>);
  }

  private static readonly STYLES = {
    username: {
      font: '500 14px Roboto',
      color: '#4B23A0',
      whiteSpace: 'nowrap' as 'nowrap'
    }
  };
}
