import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { NavigationTab, PageLayout } from '../../components';

interface Properties {

  /** The selected tab. Defaults to GENERATED. Null leaves all unselected. */
  current?: ReportPage.Tab;

  /** The reporting subpage to display. */
  children?: React.ReactNode;

  /** Requests navigation to the selected reporting section. */
  onNavigate?: (tab: ReportPage.Tab) => void;
}

/** Displays reporting navigation above the active subpage. */
export function ReportPage(props: Properties): JSX.Element {
  const current = (() => {
    if(props.current === undefined) {
      return ReportPage.Tab.GENERATED;
    }
    return props.current;
  })();
  return <div className={css(STYLES.page)}>
    <Header current={current} onNavigate={props.onNavigate}/>
    <div className={css(STYLES.content)}>{props.children}</div>
  </div>;
}

export namespace ReportPage {

  /** The reporting sections shown in the navigation menu. */
  export enum Tab {

    /** Completed reports available to view and download. */
    GENERATED,

    /** Reports being generated or awaiting retry. */
    ACTIVITY,

    /** Reports scheduled to run in the future. */
    SCHEDULES
  }
}

interface NavigationProperties {
  current: ReportPage.Tab;
  onNavigate: (tab: ReportPage.Tab) => void;
}

interface TabDefinition {
  tab: ReportPage.Tab;
  label: string;
  icon: string;
  href: string;
}

interface TabProperties extends NavigationProperties {
  definition: TabDefinition;
}

function Header(props: NavigationProperties): JSX.Element {
  return <header className={css(STYLES.header)}>
    <PageLayout>
      <Nav {...props}/>
    </PageLayout>
  </header>;
}

function Nav(props: NavigationProperties): JSX.Element {
  return <nav aria-label='Reports' className={css(STYLES.nav)}>
    <Menu {...props}/>
  </nav>;
}

function Menu(props: NavigationProperties): JSX.Element {
  return <ul className={css(STYLES.menu)}>
    {TABS.map(definition => <Li key={definition.tab}
      {...props} definition={definition}/>)}
  </ul>;
}

function Li(props: TabProperties): JSX.Element {
  return <li className={css(STYLES.item)}>
    <ResponsiveTab {...props}/>
  </li>;
}

class ResponsiveTab extends React.Component<TabProperties,
    {wide: boolean}> {
  constructor(props: TabProperties) {
    super(props);
    this.query = window.matchMedia('(min-width: 768px)');
    this.state = {wide: this.query.matches};
  }

  public render(): JSX.Element {
    const definition = this.props.definition;
    const variant = (() => {
      if(this.state.wide) {
        return NavigationTab.Variant.ICON_LABEL;
      }
      return NavigationTab.Variant.ICON;
    })();
    return <NavigationTab label={definition.label} icon={definition.icon}
      href={definition.href} variant={variant}
      isCurrent={this.props.current === definition.tab}
      onClick={this.onClick}/>;
  }

  public componentDidMount(): void {
    this.query.addEventListener('change', this.onMediaChange);
    if(this.query.matches !== this.state.wide) {
      this.setState({wide: this.query.matches});
    }
  }

  public componentWillUnmount(): void {
    this.query.removeEventListener('change', this.onMediaChange);
  }

  private onMediaChange = (event: MediaQueryListEvent) => {
    this.setState({wide: event.matches});
  };

  private onClick = () => {
    this.props.onNavigate?.(this.props.definition.tab);
  };

  private query: MediaQueryList;
}

const TABS: readonly TabDefinition[] = [
  {tab: ReportPage.Tab.GENERATED, label: 'Generated', href: '/generated',
    icon: 'resources/report_page/generated-icon.svg'},
  {tab: ReportPage.Tab.ACTIVITY, label: 'Activity', href: '/activity',
    icon: 'resources/report_page/activity-icon.svg'},
  {tab: ReportPage.Tab.SCHEDULES, label: 'Schedules', href: '/schedules',
    icon: 'resources/report_page/scheduled-icon.svg'}
];
const STYLES = StyleSheet.create({
  page: {display: 'flex', flexDirection: 'column', flex: '1 1 auto',
    minHeight: 0, width: '100%', height: '100%', overflow: 'hidden'},
  header: {flex: '0 0 auto', boxSizing: 'border-box', paddingRight: '18px',
    borderBottom: '1px solid #E6E6E6', backgroundColor: '#FFFFFF',
    '@media (width < 768px)': {paddingLeft: '11px'},
    '@media (min-width: 768px)': {paddingLeft: '3px'}},
  nav: {display: 'flex', flexDirection: 'column', alignItems: 'flex-start'},
  menu: {display: 'flex', justifyContent: 'space-between', listStyle: 'none',
    margin: 0, padding: 0,
    '@media (width < 768px)': {width: 'clamp(114px, 36%, 146px)'},
    '@media (min-width: 768px)': {width: 'fit-content'}},
  item: {display: 'flex', flex: '0 0 auto'},
  content: {display: 'flex', flexDirection: 'column', flex: '1 1 auto',
    minHeight: 0, minWidth: 0, overflow: 'hidden'}
});
