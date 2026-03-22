import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';

interface Properties {

  /** The full path for the icon. */
  icon: string;

  /** The text label to display inside the tab. */
  label: string;

  /** The URL to push to the browser history when the tab is clicked. */
  href: string;

  /** Whether this tab corresponds to the current page. Defaults to false. */
  isCurrent?: boolean;

  /** The visual representation of the tab. Defaults to ICON_LABEL. */
  variant?: NavigationTab.Variant;

  /** Called when the tab is clicked. */
  onClick?: () => void;
}

interface State {
  isHovered: boolean;
  isFocused: boolean;
}

/** Represents a navigation tab within a NavigationHeader. */
export class NavigationTab extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isHovered: false,
      isFocused: false
    };
  }

  public render(): JSX.Element {
    const variant = this.props.variant ?? NavigationTab.Variant.ICON_LABEL;
    const isHighlighted = this.props.isCurrent || this.state.isHovered ||
      this.state.isFocused;
    const iconColor = isHighlighted ? '#4B23A0' : '#7D7E90';
    const iconSize = (() => {
      if(variant === NavigationTab.Variant.ICON_LABEL) {
        return '16px';
      }
      return '24px';
    })();
    const tabClassName = (() => {
      if(variant === NavigationTab.Variant.ICON_LABEL) {
        return css(NavigationTab.STYLES.tab,
          NavigationTab.STYLES.iconLabelTab);
      }
      return css(NavigationTab.STYLES.tab, NavigationTab.STYLES.iconTab);
    })();
    const labelStyle = {
      ...NavigationTab.INLINE_STYLES.label,
      color: iconColor
    };
    return (
      <a className={tabClassName} href={this.props.href}
          aria-current={this.props.isCurrent ? 'page' : 'false'}
          onClick={this.onClick}
          onFocus={this.onFocus}
          onBlur={this.onBlur}
          onMouseEnter={this.onMouseEnter}
          onMouseLeave={this.onMouseLeave}>
        <div className={css(NavigationTab.STYLES.content)}>
          <div aria-hidden style={{
              width: iconSize,
              height: iconSize,
              backgroundColor: iconColor,
              WebkitMaskImage: `url(${this.props.icon})`,
              maskImage: `url(${this.props.icon})`,
              WebkitMaskSize: 'contain',
              maskSize: 'contain',
              WebkitMaskRepeat: 'no-repeat',
              maskRepeat: 'no-repeat'
            }}/>
          {variant === NavigationTab.Variant.ICON_LABEL &&
            <>
              <div className={css(NavigationTab.STYLES.iconLabelSpacer)}/>
              <span style={labelStyle}>{this.props.label}</span>
            </>}
        </div>
        <div className={css(NavigationTab.STYLES.indicator,
          this.props.isCurrent && NavigationTab.STYLES.activeIndicator)}/>
      </a>);
  }

  private onClick = (event: React.MouseEvent) => {
    event.preventDefault();
    this.props.onClick?.();
  }

  private onFocus = () => {
    this.setState({isFocused: true});
  }

  private onBlur = () => {
    this.setState({isFocused: false});
  }

  private onMouseEnter = () => {
    this.setState({isHovered: true});
  }

  private onMouseLeave = () => {
    this.setState({isHovered: false});
  }

  private static readonly INLINE_STYLES = {
    label: {
      fontSize: '0.875rem',
      fontFamily: 'Roboto',
      whiteSpace: 'nowrap'
    }
  };

  private static readonly STYLES = StyleSheet.create({
    tab: {
      display: 'inline-flex',
      flexDirection: 'column',
      height: '40px',
      border: '1px solid transparent',
      cursor: 'pointer',
      textDecoration: 'none',
      boxSizing: 'border-box',
      '-webkit-tap-highlight-color': 'transparent',
      outline: 'none',
      ':focus-visible': {
        borderColor: '#4B23A0'
      }
    },
    iconTab: {
      width: '38px',
      alignItems: 'center',
      justifyContent: 'center'
    },
    iconLabelTab: {
      padding: '0 15px'
    },
    content: {
      display: 'flex',
      alignItems: 'center',
      flex: '1 1 auto'
    },
    iconLabelSpacer: {
      width: '8px',
      flexShrink: 0
    },
    indicator: {
      height: '2px',
      width: '100%',
      backgroundColor: '#4B23A0',
      opacity: 0,
      flexShrink: 0
    },
    activeIndicator: {
      opacity: 1
    }
  });
}

export namespace NavigationTab {

  /** The visual representation of a NavigationTab. */
  export enum Variant {

    /** Displays only the icon. */
    ICON,

    /** Displays both the icon and the label. */
    ICON_LABEL
  }
}
