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
    return (
      <VBoxLayout width='68px' height='40px'>
        <Padding/>
        <HBoxLayout width='68px' height='14px'>
          <img src='resources/account/trader-grey.svg' width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src='resources/account/manager-grey.svg' width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src='resources/account/admin-grey.svg' width='14px'
            height='14px'/>
          <Padding size='4px'/>
          <img src='resources/account/service-grey.svg' width='14px'
            height='14px'/>
        </HBoxLayout>
        <Padding/>
      </VBoxLayout>);
  }
}
