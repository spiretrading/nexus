import * as Nexus from 'nexus';
import * as React from 'react';
import { Transition } from 'react-transition-group';
import { HBoxLayout, Padding } from '../../..';
import { RoleIcon } from './role_icon';

interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;

  /** Called when the user clicks on a role
   * @param value - The role of the icon clicked.
   */
  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesField extends React.Component<Properties, {}> {

  public render(): JSX.Element {
    return (
      <HBoxLayout width={RolesField.COMPONENT_WIDTH}
          height={RolesField.IMAGE_SIZE}>
        <RoleIcon role={Nexus.AccountRoles.Role.TRADER}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.TRADER)}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.TRADER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.MANAGER}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.MANAGER)}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.MANAGER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.ADMINISTRATOR}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.ADMINISTRATOR)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.SERVICE}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.SERVICE)}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.SERVICE)}/>
      </HBoxLayout>);
  }

  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_PADDING = '14px';
  private static readonly COMPONENT_WIDTH = '68px';
}
