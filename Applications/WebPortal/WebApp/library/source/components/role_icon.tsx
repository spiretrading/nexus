import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { Transition } from 'react-transition-group';
import { DisplaySize } from '../display_size';

interface Properties {

  /** The size of the element to display. */
  displaySize: DisplaySize;

  /** Determines if the role icon should be even smaller. */
  isExtraSmall?: boolean;

  /** The role the icon represents. */
  role: Nexus.AccountRoles.Role;

  /** Whether the roles can be changed. */
  readOnly?: boolean;

  /** Determines if the role is set. */
  isSet: boolean;

  /** Determines if the mobile tooltip should be shown. */
  isTouchTooltipShown: boolean;

  /** Called when the icon is clicked on. */
  onClick?: () => void;

  /** Called when the icon is touched. */
  onTouch?: () => void;
}

interface State {
  isMouseTooltipShown: boolean;
  tooltipPosition: { top: number, left: number };
}

/** Displays a panel of icons highlighting an account's roles. */
export class RoleIcon extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isMouseTooltipShown: false,
      tooltipPosition: { top: 0, left: 0 }
    };
    this.iconRef = React.createRef();
  }

  public render(): JSX.Element {
    const iconColor = (() => {
      if(this.props.isSet) {
        return 'purple';
      }
      return 'grey';
    })();
    const iconStyle = (() => {
      if(this.props.readOnly) {
        return RoleIcon.STYLE.readonly;
      } else {
        return RoleIcon.STYLE.clickable;
      }
    })();
    const iconSize = (() => {
      if(this.props.isExtraSmall) {
        return RoleIcon.IMAGE_SIZE_EXTRA_SMALL;
      } else {
        return RoleIcon.IMAGE_SIZE;
      }
    })();
    const iconWrapper = (() => {
      if(this.props.isExtraSmall) {
        return RoleIcon.STYLE.iconWrapperExtraSmall;
      } else if(this.props.displaySize === DisplaySize.SMALL) {
        return RoleIcon.STYLE.iconWrapperSmall;
      } else {
        return RoleIcon.STYLE.iconWrapperLarge;
      }
    })();
    const isTooltipVisible = this.props.isTouchTooltipShown ||
      (this.state.isMouseTooltipShown && !this.props.isTouchTooltipShown);
    return (
      <div ref={this.iconRef}
          style={{...iconWrapper, ...iconStyle}}
          onClick={this.onClick}
          onTouchStart={this.onTouch}
          onMouseEnter={this.showTooltipMouse}
          onMouseLeave={this.hideToolTipMouse}>
        <img src={`${this.getSource(this.props.role)}${iconColor}.svg`}
          style={iconStyle}
          width={iconSize}
          height={iconSize}/>
        {this.renderTooltip(isTooltipVisible)}
      </div>);
  }

  private renderTooltip(isVisible: boolean): React.ReactPortal {
    return ReactDOM.createPortal(
      <Transition timeout={RoleIcon.TIMEOUT_TOOLTIP} in={isVisible}>
        {(state) => (
          <div style={{...RoleIcon.STYLE.imageTooltip,
              ...RoleIcon.ANIMATION_STYLE[state],
              top: this.state.tooltipPosition.top,
              left: this.state.tooltipPosition.left}}>
            {this.getText(this.props.role)}
          </div>)}
      </Transition>,
      document.body);
  }

  private updateTooltipPosition() {
    if(this.iconRef.current) {
      var rect = this.iconRef.current.getBoundingClientRect();
      this.setState({
        tooltipPosition: {
          top: rect.bottom + 2,
          left: rect.left
        }
      });
    }
  }

  private showTooltipMouse = () => {
    if(!this.state.isMouseTooltipShown && !this.props.isTouchTooltipShown) {
      this.updateTooltipPosition();
      this.setState({isMouseTooltipShown: true});
    }
  }

  private hideToolTipMouse = () => {
    this.setState({isMouseTooltipShown: false});
  }

  private onClick = () => {
    if(!this.props.readOnly && !this.props.isTouchTooltipShown) {
      this.props.onClick?.();
    }
  }

  private onTouch = () => {
    if(!this.props.readOnly) {
      this.props.onClick?.();
    }
    this.updateTooltipPosition();
    this.props.onTouch?.();
  }

  private getText = (role: Nexus.AccountRoles.Role) => {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        return RoleIcon.TRADER_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.MANAGER:
        return RoleIcon.MANAGER_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        return RoleIcon.ADMINISTRATOR_TOOLTIP_TEXT;
      case Nexus.AccountRoles.Role.SERVICE:
        return RoleIcon.SERVICE_TOOLTIP_TEXT;
    }
  }

  private getSource = (role: Nexus.AccountRoles.Role) => {
    switch (role) {
      case Nexus.AccountRoles.Role.TRADER:
        return 'resources/account/trader-';
      case Nexus.AccountRoles.Role.MANAGER:
        return 'resources/account/manager-';
      case Nexus.AccountRoles.Role.ADMINISTRATOR:
        return 'resources/account/admin-';
      case Nexus.AccountRoles.Role.SERVICE:
        return 'resources/account/service-';
    }
  }

  private iconRef: React.RefObject<HTMLDivElement>;
  private static readonly ANIMATION_STYLE: Record<string, React.CSSProperties> = {
    entering: {
      opacity: 0
    },
    entered: {
      opacity: 1
    },
    exiting: {
      opacity: 0
    },
    exited: {
      opacity: 0,
      pointerEvents: 'none'
    }
  };
  private static readonly STYLE: Record<string, React.CSSProperties> = {
    iconWrapperLarge: {
      boxSizing: 'border-box',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      height: '24px',
      width: '24px',
      padding: '2px',
      outline: 0
    },
    iconWrapperSmall: {
      boxSizing: 'border-box',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      height: '34px',
      width: '34px',
      padding: '7px',
      outline: 0
    },
    iconWrapperExtraSmall: {
      boxSizing: 'border-box',
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      height: '14px',
      width: '14px',
      padding: 0,
      outline: 0
    },
    clickable: {
      cursor: 'pointer'
    },
    readonly: {
      cursor: 'inherit'
    },
    imageTooltip: {
      display: 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      font: '400 12px Roboto',
      paddingLeft: '15px',
      paddingRight: '15px',
      height: '22px',
      backgroundColor: '#4B23A0',
      color: '#FFFFFF',
      position: 'fixed',
      border: '1px solid #4B23A0',
      borderRadius: '1px',
      boxShadow: '0px 0px 2px #00000064',
      zIndex: 10000,
      whiteSpace: 'nowrap',
      opacity: 0,
      transition: 'opacity 200ms ease-in-out',
      pointerEvents: 'none'
    }
  };
  private static readonly TIMEOUT_TOOLTIP = {
    enter: 100,
    entered: 200,
    exit: 200,
    exited: 1
  };
  private static readonly IMAGE_SIZE = '20px';
  private static readonly IMAGE_SIZE_EXTRA_SMALL = '14px';
  private static readonly TRADER_TOOLTIP_TEXT = 'Trader';
  private static readonly MANAGER_TOOLTIP_TEXT = 'Manager';
  private static readonly ADMINISTRATOR_TOOLTIP_TEXT = 'Admin';
  private static readonly SERVICE_TOOLTIP_TEXT = 'Service';
}
