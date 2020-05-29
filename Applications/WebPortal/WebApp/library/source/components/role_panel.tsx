import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolePanel extends React.Component<Properties> {
  public render(): JSX.Element {
    const getIconColour = (role: Nexus.AccountRoles.Role) => {
      if(this.props.roles.test(role)) {
        return 'purple';
      }
      return 'grey';
    };
    return (
      <div style={RolePanel.STYLE.containerStyle}>
        <img src={`/resources/account/trader-${
          getIconColour(Nexus.AccountRoles.Role.TRADER)}.svg`}
          width='14px'
          height='14px'/>
        <img src={`/resources/account/manager-${
          getIconColour(Nexus.AccountRoles.Role.MANAGER)}.svg`}
          width='14px'
          height='14px'/>
        <img src={`/resources/account/admin-${
          getIconColour(Nexus.AccountRoles.Role.ADMINISTRATOR)}.svg`}
          width='14px'
          height='14px'/>
        <img src={`/resources/account/service-${
          getIconColour(Nexus.AccountRoles.Role.SERVICE)}.svg`}
          width='14px'
          height='14px'/>
      </div>);
  }

  private static readonly STYLE = {
    containerStyle: {
      width: '100%',
      height: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      flexGrow: 0,
      flexShrink: 0,
      cursor: 'inherit' as 'inherit'
    }
  };
}
