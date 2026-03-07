import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { NavigationHeader } from '../../components/navigation_header';
import { NavigationTab } from '../../components/navigation_tab';
import { RequestDirectoryPage } from './request_directory_page';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** The currently displayed page. Defaults to YOUR_REQUESTS. */
  current?: RequestsPage.Page;

  /** The status of the displayed requests list. */
  displayStatus: RequestDirectoryPage.DisplayStatus;

  /** The currently selected request state facet. */
  requestState: RequestDirectoryPage.RequestState;

  /** The user's filter criteria. */
  filters: RequestDirectoryPage.Filters;

  /** The number of filters currently active. */
  filterCount: number;

  /** The current 0-based page index. */
  pageIndex: number;

  /** The response from the server. */
  response: RequestDirectoryPage.Response;

  /** Called when the user requests to retrieve requests. */
  onSubmit?: (submission: RequestDirectoryPage.Submission) => void;

  /** Called when a navigation tab is clicked, passing the page. */
  onNavigate?: (page: RequestsPage.Page) => void;
}

interface State {
  variant: NavigationTab.Variant;
}

/** Displays the Requests Page. */
export class RequestsPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.mediaQuery = window.matchMedia('(min-width: 768px)');
    this.state = {
      variant: this.mediaQuery.matches ?
        NavigationTab.Variant.ICON_LABEL : NavigationTab.Variant.ICON
    };
  }

  public componentDidMount(): void {
    this.mediaQuery.addEventListener('change', this.onMediaChange);
  }

  public componentWillUnmount(): void {
    this.mediaQuery.removeEventListener('change', this.onMediaChange);
  }

  public render(): JSX.Element {
    const content = this.renderContent();
    if(this.props.roles.test(Nexus.AccountRoles.Role.MANAGER) ||
        this.props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR)) {
      return (
        <div className={css(RequestsPage.STYLES.container)}>
          <NavigationHeader variant={this.state.variant}
              className={css(RequestsPage.STYLES.header)}
              current={RequestsPage.toHref(this.props.current !== undefined ?
                this.props.current : RequestsPage.Page.YOUR_REQUESTS)}
              onNavigate={this.onNavigate}>
            <NavigationTab
              icon='resources/requests_page/your-requests${highlight}.svg'
              label='Your Requests'
              href='requests/you'/>
            <NavigationTab
              icon='resources/requests_page/group-requests${highlight}.svg'
              label='Group Requests'
              href='requests/group'/>
          </NavigationHeader>
          {content}
        </div>);
    }
    return content;
  }

  private renderContent(): JSX.Element {
    const current = this.props.current ?? RequestsPage.Page.YOUR_REQUESTS;
    const scope = current === RequestsPage.Page.YOUR_REQUESTS ?
      RequestDirectoryPage.Scope.YOU : RequestDirectoryPage.Scope.GROUP;
    return (
      <RequestDirectoryPage
        scope={scope}
        displayStatus={this.props.displayStatus}
        requestState={this.props.requestState}
        filters={this.props.filters}
        filterCount={this.props.filterCount}
        pageIndex={this.props.pageIndex}
        response={this.props.response}
        onSubmit={this.props.onSubmit}/>);
  }

  private onNavigate = (href: string) => {
    const page = RequestsPage.toPage(href);
    if(page !== undefined) {
      this.props.onNavigate?.(page);
    }
  }

  private onMediaChange = (event: MediaQueryListEvent) => {
    this.setState({
      variant: event.matches ?
        NavigationTab.Variant.ICON_LABEL : NavigationTab.Variant.ICON
    });
  }

  private mediaQuery: MediaQueryList;

  private static toHref(page: RequestsPage.Page): string {
    switch(page) {
      case RequestsPage.Page.YOUR_REQUESTS:
        return 'requests/you';
      case RequestsPage.Page.GROUP_REQUESTS:
        return 'requests/group';
    }
  }

  private static toPage(href: string): RequestsPage.Page | undefined {
    switch(href) {
      case 'requests/you':
        return RequestsPage.Page.YOUR_REQUESTS;
      case 'requests/group':
        return RequestsPage.Page.GROUP_REQUESTS;
      default:
        return undefined;
    }
  }

  private static readonly STYLES = StyleSheet.create({
    container: {
      overflow: 'hidden'
    },
    header: {
      borderBottom: '1px solid #E6E6E6',
      paddingLeft: '11px',
      paddingRight: '18px',
      backgroundColor: '#FFFFFF',
      '@media (min-width: 768px)': {
        paddingLeft: '3px'
      },
      '> nav': {
        width: 'clamp(76px, 24%, 92px)',
        '@media (min-width: 768px)': {
          width: 'auto'
        }
      }
    }
  });
}

export namespace RequestsPage {

  /** The pages within the Requests Page. */
  export enum Page {

    /** The user's own requests. */
    YOUR_REQUESTS,

    /** The group's requests. */
    GROUP_REQUESTS
  }
}
