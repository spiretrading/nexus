import * as React from 'react';
import { matchPath, RouteComponentProps } from 'react-router-dom';
import { DisplaySize } from '../../display_size';
import { PageNotFoundPage } from '../page_not_found_page';
import { ReportController } from './report_controller';
import { ReportModel } from './report_model';

interface Properties extends RouteComponentProps {

  /** The model providing the reporting subpage models. */
  model: ReportModel;

  /** The device's display size. */
  displaySize: DisplaySize;
}

/** Connects reporting subpages and links to browser navigation. */
export class ReportRouter extends React.Component<Properties> {
  public render(): JSX.Element {
    const route = matchReport(this.props.location.pathname);
    if(!route) {
      return <PageNotFoundPage displaySize={this.props.displaySize}/>;
    }
    return <div onClick={this.onClick} style={{display: 'flex',
        flexDirection: 'column', flex: '1 1 auto', minHeight: 0,
        overflow: 'hidden'}}>
      <ReportController model={this.props.model} page={route.page}
        reportId={route.id} scheduleId={route.id} basePath='/reports'
        onNewReport={this.onNewReport} onNavigate={this.onNavigate}
        onActionError={this.onActionError}/>
    </div>;
  }

  private navigate(path: string): void {
    const current = this.props.location;
    if(path !== current.pathname + current.search + current.hash) {
      this.props.history.push(path);
    }
  }

  private onNewReport = () => {
    this.navigate('/reports/new');
  };

  private onNavigate = (page: ReportController.Page) => {
    const path = PATHS.get(page);
    if(path) {
      this.navigate(path);
    }
  };

  private onClick = (event: React.MouseEvent<HTMLDivElement>) => {
    if(event.defaultPrevented || event.button !== 0 || event.ctrlKey ||
        event.metaKey || event.shiftKey || event.altKey) {
      return;
    }
    const anchor = (event.target as Element).closest('a[href]');
    if(!anchor || anchor.hasAttribute('download') ||
        anchor.getAttribute('target') && anchor.getAttribute('target') !==
          '_self') {
      return;
    }
    const url = new URL(anchor.getAttribute('href'), window.location.href);
    if(url.origin !== window.location.origin || !matchReport(url.pathname)) {
      return;
    }
    event.preventDefault();
    this.navigate(url.pathname + url.search + url.hash);
  };

  private onActionError = (error: unknown) => {
    console.error(error);
  };
}

function matchReport(path: string): {page: ReportController.Page; id?: string} {
  for(const [page, url] of PATHS) {
    if(matchPath(path, {path: url, exact: true})) {
      return {page};
    }
  }
  if(matchPath(path, {path: '/reports', exact: true})) {
    return {page: ReportController.Page.GENERATED};
  }
  if(matchPath(path, {path: '/reports/edit', exact: true})) {
    return null;
  }
  const edit = matchPath<{id: string}>(path,
    {path: '/reports/edit/:id', exact: true});
  const detail = edit || matchPath<{id: string}>(path,
    {path: '/reports/:id', exact: true});
  if(!detail) {
    return null;
  }
  try {
    const id = decodeURIComponent(detail.params.id);
    if(edit) {
      return {page: ReportController.Page.EDIT_SCHEDULED, id};
    }
    return {page: ReportController.Page.DETAIL, id};
  } catch {
    return null;
  }
}

const PATHS = new Map([
  [ReportController.Page.GENERATED, '/reports/generated'],
  [ReportController.Page.ACTIVITY, '/reports/activity'],
  [ReportController.Page.SCHEDULED, '/reports/schedules'],
  [ReportController.Page.CREATE, '/reports/new']
]);
