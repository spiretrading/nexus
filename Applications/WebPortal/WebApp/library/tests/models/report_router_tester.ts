import { StyleSheetTestUtils } from 'aphrodite/no-important';
import * as Beam from 'beam';
import { createMemoryHistory } from 'history';
import * as assert from 'node:assert/strict';
import { after, before, describe, it } from 'node:test';
import * as Nexus from 'nexus';
import * as React from 'react';
import { renderToStaticMarkup } from 'react-dom/server';
import { NavigationTab } from '../../source/components/navigation_tab';
import { DisplaySize } from '../../source/display_size';
import { DashboardController, HttpDashboardModel, LocalAccountDirectoryModel,
  LocalDashboardModel, LocalRequestsModel, PageNotFoundPage } from
  '../../source/pages';
import { SideMenu } from '../../source/pages/dashboard_page/side_menu';
import { CompositeReportModel, HttpReportModel, ReportController, ReportPage,
  ReportRouter } from '../../source/pages/report_page';

function makeReports(): CompositeReportModel {
  return new CompositeReportModel(null, null, null, null, null, null);
}

function makeLocal(reports: CompositeReportModel): LocalDashboardModel {
  return new LocalDashboardModel(Beam.DirectoryEntry.makeAccount(1, 'Alice'),
    new Nexus.AccountRoles(0), new Nexus.EntitlementDatabase(),
    new Nexus.CountryDatabase(), new Nexus.CurrencyDatabase(),
    new Nexus.VenueDatabase(), new LocalAccountDirectoryModel(new Beam.Map()),
    new LocalRequestsModel(Beam.DirectoryEntry.INVALID, [], new Map()),
    reports);
}

function find(element: React.ReactElement,
    predicate: (element: any) => boolean): React.ReactElement {
  if(predicate(element)) {
    return element;
  }
  for(const child of React.Children.toArray(element.props.children)) {
    if(React.isValidElement(child)) {
      const match = find(child, predicate);
      if(match) {
        return match;
      }
    }
  }
  return null;
}

function makeRouter(path: string) {
  const history = createMemoryHistory({initialEntries: [path]});
  const props = {history, get location() { return history.location; },
    match: {params: {}, path: '/reports', url: '/reports', isExact: false},
    model: makeReports(), displaySize: DisplaySize.LARGE};
  const router = new ReportRouter(props);
  return {router, history};
}

function click(href: string, options: Record<string, unknown> = {},
    attributes: Record<string, string> = {}) {
  const event = {button: 0, defaultPrevented: false,
    preventDefault: () => { event.defaultPrevented = true; },
    target: {closest: () => ({
      getAttribute: (name: string) => ({href, ...attributes})[name],
      hasAttribute: (name: string) => name in attributes
    })}, ...options};
  return event;
}

describe('Report routing', () => {
  const originalWindow = globalThis.window;
  before(() => {
    StyleSheetTestUtils.suppressStyleInjection();
    globalThis.window = {location: new URL('https://portal.test/reports'),
      matchMedia: () => ({matches: true})} as unknown as Window &
        typeof globalThis;
  });
  after(() => {
    globalThis.window = originalWindow;
    StyleSheetTestUtils.clearBufferAndResumeStyleInjection();
  });

  it('resolves_all_subpages_and_decodes_route_ids', () => {
    const cases: [string, ReportController.Page, string?][] = [
      ['/reports', ReportController.Page.GENERATED],
      ['/reports/generated', ReportController.Page.GENERATED],
      ['/reports/activity', ReportController.Page.ACTIVITY],
      ['/reports/schedules', ReportController.Page.SCHEDULED],
      ['/reports/new', ReportController.Page.CREATE],
      ['/reports/edit/job%20one', ReportController.Page.EDIT_SCHEDULED,
        'job one'],
      ['/reports/job%2Fone', ReportController.Page.DETAIL, 'job/one']];
    for(const [path, page, id] of cases) {
      const {router} = makeRouter(path);
      const child = router.render().props.children;
      assert.equal(child.type, ReportController);
      assert.equal(child.props.page, page);
      assert.equal(child.props.reportId, id);
      assert.equal(child.props.scheduleId, id);
      assert.equal(child.props.basePath, '/reports');
      assert.equal(child.props.model, router.props.model);
    }
    for(const path of ['/reports/edit', '/reports/edit/', '/reports/a/b']) {
      assert.equal(makeRouter(path).router.render().type, PageNotFoundPage);
    }
  });

  it('updates_history_immediately_and_supports_back_and_forward', () => {
    const {router, history} = makeRouter('/reports');
    const controller = router.render().props.children.props;
    controller.onNewReport();
    assert.equal(history.location.pathname, '/reports/new');
    controller.onNavigate(ReportController.Page.ACTIVITY);
    assert.equal(history.location.pathname, '/reports/activity');
    controller.onNavigate(ReportController.Page.ACTIVITY);
    assert.equal(history.length, 3);
    history.goBack();
    assert.equal(router.render().props.children.props.page,
      ReportController.Page.CREATE);
    history.goBack();
    assert.equal(router.render().props.children.props.page,
      ReportController.Page.GENERATED);
    history.goForward();
    assert.equal(router.render().props.children.props.page,
      ReportController.Page.CREATE);
    controller.onNavigate(ReportController.Page.SCHEDULED);
    assert.equal(history.location.pathname, '/reports/schedules');
  });

  it('routes_report_links_and_preserves_native_link_actions', () => {
    const {router, history} = makeRouter('/reports');
    const onClick = router.render().props.onClick;
    const detail = click('/reports/job?view=raw#section');
    onClick(detail);
    assert.ok(detail.defaultPrevented);
    assert.equal(history.location.pathname, '/reports/job');
    assert.equal(history.location.search, '?view=raw');
    assert.equal(history.location.hash, '#section');
    const edit = click('/reports/edit/job');
    onClick(edit);
    assert.ok(edit.defaultPrevented);
    assert.equal(history.location.pathname, '/reports/edit/job');
    for(const event of [click('/reports/job', {ctrlKey: true}),
        click('/reports/job', {metaKey: true}),
        click('/reports/job', {shiftKey: true}),
        click('/reports/job', {altKey: true}),
        click('/reports/job', {button: 1}),
        click('/reports/job', {}, {target: '_blank'}),
        click('/reports/job', {}, {download: ''}),
        click('https://elsewhere.test/reports/job'),
        click('/api/reporting_service/download_report?id=job'),
        click('/reports/not/a/route')]) {
      onClick(event);
      assert.equal(event.defaultPrevented, false);
      assert.equal(history.location.pathname, '/reports/edit/job');
    }
  });

  it('uses_dashboard_urls_in_tabs_and_keeps_standalone_defaults', () => {
    const production = renderToStaticMarkup(React.createElement(ReportPage,
      {basePath: '/reports'}));
    for(const path of ['generated', 'activity', 'schedules']) {
      assert.ok(production.includes(`href="/reports/${path}"`));
    }
    const standalone = renderToStaticMarkup(React.createElement(ReportPage));
    assert.ok(standalone.includes('href="/generated"'));
    let navigations = 0;
    const tab = new NavigationTab({label: 'Reports', icon: 'icon.svg',
      href: '/reports', onClick: () => { ++navigations; }});
    const onClick = tab.render().props.onClick;
    const modified = click('/reports', {ctrlKey: true});
    onClick(modified);
    assert.equal(modified.defaultPrevented, false);
    assert.equal(navigations, 0);
    const plain = click('/reports');
    onClick(plain);
    assert.ok(plain.defaultPrevented);
    assert.equal(navigations, 1);
  });

  it('injects_dashboard_report_model_without_loading_subpages', async () => {
    const reports = makeReports();
    const model = makeLocal(reports);
    assert.throws(() => model.reportModel, /not loaded/);
    await model.load();
    assert.equal(model.reportModel, reports);
    assert.equal(reports.isLoaded, false);
    const {router} = makeRouter('/reports/activity');
    const dashboard = new DashboardController({...router.props, model});
    Object.assign(dashboard.state, {isLoaded: true});
    const route = find(dashboard.render(), entry =>
      entry.props.path === '/reports');
    const routed = route.props.render(router.props);
    assert.equal(routed.type, ReportRouter);
    assert.equal(routed.props.model, reports);
    const menu = new SideMenu({roles: new Nexus.AccountRoles(0)});
    assert.ok(find(menu.render(), entry => entry.props.href === '/reports'));
  });

  it('composes_http_reports_after_login_without_fetching_report_data',
      async () => {
    const account = Beam.DirectoryEntry.makeAccount(1, 'Alice');
    let opened = 0;
    const clients = {open: async () => { ++opened; },
      serviceLocatorClient: {loadCurrentAccount: async () => account},
      administrationClient: {
        loadAccountRoles: async () => new Nexus.AccountRoles(0),
        loadNotifications: async () => [] as Nexus.Notification[],
        monitorNotifications: () => {}},
      definitionsClient: {entitlementDatabase: new Nexus.EntitlementDatabase(),
        countryDatabase: new Nexus.CountryDatabase(),
        currencyDatabase: new Nexus.CurrencyDatabase(),
        venueDatabase: new Nexus.VenueDatabase()}};
    const model = new HttpDashboardModel(
      clients as unknown as Nexus.ServiceClients);
    assert.throws(() => model.reportModel, /not loaded/);
    await model.load();
    assert.equal(opened, 1);
    const reports = model.reportModel;
    assert.ok(reports instanceof HttpReportModel);
    assert.equal(reports.isLoaded, false);
    await model.load();
    assert.equal(opened, 1);
    assert.equal(model.reportModel, reports);
  });
});
