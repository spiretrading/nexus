import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { NavigationTab } from './navigation_tab';

interface Properties {

  /** The visual representation applied to each tab. Defaults to ICON_LABEL. */
  variant?: NavigationTab.Variant;

  /** The href of the currently selected tab. */
  current?: string;

  /** Called when a tab is clicked, passing the tab's href. */
  onNavigate?: (href: string) => void;
}

/** Displays a navigation header with tabs. */
export class NavigationHeader extends React.Component<Properties> {
  public render(): JSX.Element {
    const children = React.Children.map(this.props.children,
      (child, index) => {
        if(!React.isValidElement(child) || child.type !== NavigationTab) {
          return child;
        }
        const tabProps = child.props as {href: string};
        return (
          <>
            {index > 0 &&
              <div className={css(NavigationHeader.STYLES.gap)}/>}
            {React.cloneElement(child, {
              variant: this.props.variant,
              isCurrent: tabProps.href === this.props.current,
              onClick: () => this.props.onNavigate?.(tabProps.href)
            })}
          </>);
      });
    return (
      <header className={css(NavigationHeader.STYLES.header)}>
        {children}
      </header>);
  }

  private static readonly STYLES = StyleSheet.create({
    header: {
      display: 'inline-flex',
      alignItems: 'center' as 'center',
      height: '40px'
    },
    gap: {
      width: '30px'
    }
  });
}
