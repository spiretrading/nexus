import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, RolesField } from '../../..';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The selected roles. */
  roles: Nexus.AccountRoles;

  /** The id of the label naming the group of roles. */
  labelId?: string;

  /** Called when the user clicks on a role
   * @param value - The role of the icon clicked.
   */
  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

export class RolesInput extends React.Component<Properties> {
  public render(): JSX.Element {
    return (
      <div style={RolesInput.STYLE.container}
          role='group'
          aria-labelledby={this.props.labelId}>
        <RolesField
          displaySize={this.props.displaySize}
          roles={this.props.roles}
          onClick={this.props.onClick}/>
      </div>);
  }

  private static readonly STYLE = {
    container: {
      display: 'flex',
      flexWrap: 'nowrap',
      width: '100%',
      flexDirection: 'row',
      alignItems: 'center',
      justifyContent: 'space-between'
    } as React.CSSProperties
  }
}
