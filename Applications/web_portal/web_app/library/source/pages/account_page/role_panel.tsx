import * as Nexus from 'nexus';
import * as React from 'react';
import {HBoxLayout, Padding, VBoxLayout} from '../..';
interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolePanel extends React.Component<Properties> {
  public render(): JSX.Element {
    const getIconColour = (role: Nexus.AccountRoles.Role) => {
      if(this.props.roles.isSet(role)) {
        return 'purple';
      }
      return 'grey';
    };
    return (
      <VBoxLayout width='68px' height='40px'>
        <Padding/>
        <HBoxLayout width='68px' height='14px'>
          <img src={
            `resources/account/trader-${
              getIconColour(Nexus.AccountRoles.Role.TRADER)}.svg`}
            width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src={`resources/account/manager-${
              getIconColour(Nexus.AccountRoles.Role.MANAGER)}.svg`}
            width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src={`resources/account/admin-${
              getIconColour(Nexus.AccountRoles.Role.ADMINISTRATOR)}.svg`}
            width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src={`resources/account/service-${
              getIconColour(Nexus.AccountRoles.Role.SERVICE)}.svg`}
            width='14px'
            height='14px'/>
        </HBoxLayout>
        <Padding/>
      </VBoxLayout>);
  }
}
