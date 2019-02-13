import { HBoxLayout, Padding } from 'dali';
import * as Nexus from 'nexus';
import * as React from 'react';
import { RoleIcon } from './role_icon';

interface Properties {

  /** The roles to highlight. */
  roles: Nexus.AccountRoles;

  /** Whether the roles can be selected. */
  readonly?: boolean;

  /** Called when the user clicks on a role
   * @param value - The role of the icon clicked.
   */
  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

/** Displays a panel of icons highlighting an account's roles. */
export class RolesField extends React.Component<Properties, {}> {
  public static readonly defaultProps = {
    readonly: false,
    onClick: () => {}
  }

  public render(): JSX.Element {
    const Icon = (props: any) => {
      return (<RoleIcon role={props.role}
        readonly={this.props.readonly}
        isSet={this.props.roles.test(props.role)}
        onClick={() => this.props.onClick(props.role)}/>);
    };
    return (
      <div style={RolesField.STYLE.containerStyle}>
        <Icon role={Nexus.AccountRoles.Role.TRADER}/>
        <Icon role={Nexus.AccountRoles.Role.MANAGER}/>
        <Icon role={Nexus.AccountRoles.Role.ADMINISTRATOR}/>
        <Icon role={Nexus.AccountRoles.Role.SERVICE}/>
      </div>);
  }

  public static readonly STYLE = {
    containerStyle: {
      width: '100%',
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      justifyContent: 'space-between' as 'space-between',
      alignItems: 'center' as 'center',
      flexGrow: 0,
      flexShrink: 0
    }
  };
}
