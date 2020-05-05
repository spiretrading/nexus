import { HBoxLayout, Padding } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize } from '../../..';
import { RoleIcon } from './role_icon';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;

  /** Whether the roles can be selected. */
  readonly?: boolean;

  /** Called when the user clicks on a role
   * @param value - The role of the icon clicked.
   */
  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

interface State {
  mobileTooltipRole: Nexus.AccountRoles.Role;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    readonly: false,
    onClick: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      mobileTooltipRole: null
    };
    this.onTouchTooltipEvent = this.onTouchTooltipEvent.bind(this);
  }

  public render(): JSX.Element {
    return (
      <HBoxLayout width={RolesField.COMPONENT_WIDTH}
          height={RolesField.IMAGE_SIZE}>
        <RoleIcon role={Nexus.AccountRoles.Role.TRADER}
          displaySize={this.props.displaySize}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.TRADER)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.TRADER}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.TRADER)}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.TRADER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.MANAGER}
          displaySize={this.props.displaySize}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.MANAGER)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.MANAGER}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.MANAGER)}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.MANAGER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.ADMINISTRATOR}
          displaySize={this.props.displaySize}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.ADMINISTRATOR}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.ADMINISTRATOR)}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.ADMINISTRATOR)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.SERVICE}
          displaySize={this.props.displaySize}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.SERVICE)}
          isTouchTooltipShown={this.state.mobileTooltipRole ===
            Nexus.AccountRoles.Role.SERVICE}
          onTouch={() =>
            this.onTouchTooltipEvent(Nexus.AccountRoles.Role.SERVICE)}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.SERVICE)}/>
      </HBoxLayout>);
  }

  private onTouchTooltipEvent(role: Nexus.AccountRoles.Role) {
    this.setState({mobileTooltipRole: role});
    clearTimeout(this.timerID);
    this.timerID = setTimeout(() => {
      this.setState({mobileTooltipRole: null});}, 1500);
  }

  private timerID: NodeJS.Timeout;
  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_PADDING = '10px';
  private static readonly COMPONENT_WIDTH = '122px';
}
