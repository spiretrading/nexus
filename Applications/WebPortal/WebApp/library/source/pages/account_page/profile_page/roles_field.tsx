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
interface State {
  tooltipRole: Nexus.AccountRoles.Role;
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
      tooltipRole: null
    };
    this.onMobileTooltip = this.onMobileTooltip.bind(this);
  }

  public render(): JSX.Element {
    return (
      <HBoxLayout width={RolesField.COMPONENT_WIDTH}
          height={RolesField.IMAGE_SIZE}>
        <RoleIcon role={Nexus.AccountRoles.Role.TRADER}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.TRADER)}
          showToolTipMobile={this.state.tooltipRole === Nexus.AccountRoles.Role.TRADER}
          showMobileCallback={this.onMobileTooltip}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.TRADER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.MANAGER}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.MANAGER)}
          showToolTipMobile={this.state.tooltipRole === Nexus.AccountRoles.Role.MANAGER}
          showMobileCallback={this.onMobileTooltip}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.MANAGER)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.ADMINISTRATOR}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)}
          showToolTipMobile={this.state.tooltipRole === Nexus.AccountRoles.Role.ADMINISTRATOR}
          showMobileCallback={this.onMobileTooltip}
          onClick={() =>
            this.props.onClick(Nexus.AccountRoles.Role.ADMINISTRATOR)}/>
        <Padding size={RolesField.IMAGE_PADDING}/>
        <RoleIcon role={Nexus.AccountRoles.Role.SERVICE}
          readonly={this.props.readonly}
          isSet={this.props.roles.test(Nexus.AccountRoles.Role.SERVICE)}
          showToolTipMobile={this.state.tooltipRole === Nexus.AccountRoles.Role.SERVICE}
          showMobileCallback={this.onMobileTooltip}
          onClick={() => this.props.onClick(Nexus.AccountRoles.Role.SERVICE)}/>
      </HBoxLayout>
     );
  }

  private onMobileTooltip(role: Nexus.AccountRoles.Role, value: boolean) {
    if(value)  {
      this.setState({tooltipRole: role});
    } else if (role === this.state.tooltipRole) {
      this.setState({tooltipRole: null});
    }
  }

  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_PADDING = '10px';
  private static readonly COMPONENT_WIDTH = '122px';
}
