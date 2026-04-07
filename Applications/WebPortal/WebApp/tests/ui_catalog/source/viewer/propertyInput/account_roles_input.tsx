import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The value of the field. */
  value?: Nexus.AccountRoles;

  /** The callback to update the value. */
  update?: (newValue: Nexus.AccountRoles) => void;
}

/** An input for toggling account roles. */
export class AccountRolesInput extends React.Component<Properties> {
  public render(): JSX.Element {
    const roles = this.props.value ?? new Nexus.AccountRoles();
    return (
      <div style={AccountRolesInput.STYLE.container}>
        {ROLE_ENTRIES.map(([role, label]) =>
          <label key={role} style={AccountRolesInput.STYLE.label}>
            <input type='checkbox' checked={roles.test(role)}
              onChange={() => this.onToggle(role)}/>
            {label}
          </label>)}
      </div>);
  }

  private onToggle = (role: Nexus.AccountRoles.Role) => {
    const roles = (this.props.value ?? new Nexus.AccountRoles()).clone();
    if(roles.test(role)) {
      roles.unset(role);
    } else {
      roles.set(role);
    }
    this.props.update(roles);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex',
      flexDirection: 'column',
      gap: '4px'
    } as React.CSSProperties,
    label: {
      display: 'flex',
      alignItems: 'center',
      gap: '4px',
      cursor: 'pointer'
    } as React.CSSProperties
  };
}

const ROLE_ENTRIES: [Nexus.AccountRoles.Role, string][] = [
  [Nexus.AccountRoles.Role.TRADER, 'Trader'],
  [Nexus.AccountRoles.Role.MANAGER, 'Manager'],
  [Nexus.AccountRoles.Role.SERVICE, 'Service'],
  [Nexus.AccountRoles.Role.ADMINISTRATOR, 'Administrator']
];
