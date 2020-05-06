import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, RolesField } from '../../..';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** The selected roles. */
  roles: Nexus.AccountRoles;

  /** Is there an error. */
  isError?: boolean;

  /** Called when the user clicks on a role
   * @param value - The role of the icon clicked.
   */
  onClick?: (role: Nexus.AccountRoles.Role) => void;
}

export class RolesInput extends React.Component<Properties> {
  public static readonly defaultProps = {
    onClick: () => {}
  };

  public render(): JSX.Element {
    const error = (() => {
      if(this.props.isError) {
        return (
          <div style={RolesInput.STYLE.errorText}>
            {RolesInput.ERROR_TEXT}
          </div>);
      } else {
        return null;
      }
    })();
    return (
      <div style={RolesInput.STYLE.container}>
        <RolesField
          displaySize={this.props.displaySize}
          roles={this.props.roles}
          onClick={this.props.onClick}/>
        {error}
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
    } as React.CSSProperties,
    errorText: {
      font: '400 14px Roboto',
      color: '#E63F44',
    } as React.CSSProperties
  }
  private static readonly ERROR_TEXT = 'Select roles(s)';
}
