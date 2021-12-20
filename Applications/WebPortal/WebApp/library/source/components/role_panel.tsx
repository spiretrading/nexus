import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { RoleIcon } from './role_icon';

interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;
}

interface State {
  mobileTooltipRole: Nexus.AccountRoles.Role;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolePanel extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      mobileTooltipRole: null
    };
  }

  public render(): JSX.Element {
    return (
      <div style={RolePanel.STYLE.containerStyle}>
        <RoleIcon
          displaySize={DisplaySize.getDisplaySize()}
          isExtraSmall
          readonly
          role={Nexus.AccountRoles.Role.TRADER}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.TRADER)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.TRADER}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.TRADER)}/>
        <RoleIcon
          displaySize={DisplaySize.getDisplaySize()}
          isExtraSmall
          readonly
          role={Nexus.AccountRoles.Role.MANAGER}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.MANAGER)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.MANAGER}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.MANAGER)}/>
        <RoleIcon
          displaySize={DisplaySize.getDisplaySize()}
          isExtraSmall
          readonly
          role={Nexus.AccountRoles.Role.ADMINISTRATOR}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.ADMINISTRATOR}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.ADMINISTRATOR)}/>
        <RoleIcon
          displaySize={DisplaySize.getDisplaySize()}
          isExtraSmall
          readonly
          role={Nexus.AccountRoles.Role.SERVICE}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.SERVICE)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.SERVICE}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.SERVICE)}/>
      </div>);
  }

  private onTouchTooltipEvent = (role: Nexus.AccountRoles.Role) => {
    this.setState({mobileTooltipRole: role});
    clearTimeout(this.timerID);
    this.timerID = setTimeout(() => {
      this.setState({mobileTooltipRole: null});}, 1500);
  }

  private timerID: NodeJS.Timeout;

  private static readonly STYLE = {
    containerStyle: {
      width: '100%',
      height: '100%',
      display: 'flex',
      flexDirection: 'row',
      flexWrap: 'nowrap',
      justifyContent: 'space-between',
      alignItems: 'center',
      flexGrow: 0,
      flexShrink: 0,
      cursor: 'inherit'
    } as React.CSSProperties
  };
}
