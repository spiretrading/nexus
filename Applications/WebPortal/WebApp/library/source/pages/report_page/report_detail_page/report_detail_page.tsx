import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ButtonLink, ErrorMessage, PageLayout, Skeleton } from
  '../../../components';

interface Properties {

  /** The report heading. */
  title: string;

  /** The report parameters in display order. */
  parameters: readonly ReportDetailPage.Parameter[];

  /** The URL from which the generated file can be downloaded. */
  filePath: string;

  /** The report viewer supplied by the host application. */
  content?: React.ReactNode;

  /** The displayed report status. Defaults to READY. */
  status?: ReportDetailPage.Status;

  /** Called when the user requests another retrieval attempt. */
  onRetry?: () => void;
}

/** Displays a generated report, its parameters, and its download link. */
export function ReportDetailPage(props: Properties): JSX.Element {
  return <PageLayout>
    <Main {...props}/>
  </PageLayout>;
}

export namespace ReportDetailPage {

  /** The state displayed while retrieving a report. */
  export enum Status {

    /** The report is being retrieved. */
    IN_PROGRESS,

    /** The report could not be retrieved. */
    ERROR,

    /** The report is available. */
    READY
  }

  /** A report parameter and its formatted display value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }
}

function Main(props: Properties): JSX.Element {
  if(props.status === ReportDetailPage.Status.ERROR) {
    return <main className={css(STYLES.main)}>
      <ErrorMessage message='There was an error loading the report.'
        onRetry={props.onRetry}/>
    </main>;
  }
  return <main className={css(STYLES.main)}>
    <ReportContent {...props}/>
  </main>;
}

function ReportContent(props: Properties): JSX.Element {
  const loading = props.status === ReportDetailPage.Status.IN_PROGRESS;
  return <section aria-label='Report' aria-live='polite' aria-busy={loading}
      className={css(STYLES.reportContent)}>
    <Header {...props}/>
    <div className={css(STYLES.content)}>{!loading && props.content}</div>
    <ActionSheet filePath={props.filePath} disabled={loading}/>
  </section>;
}

function Header(props: Properties): JSX.Element {
  if(props.status === ReportDetailPage.Status.IN_PROGRESS) {
    return <header>
      <div className={css(STYLES.titleRow)} style={{height: '34px'}}>
        <Skeleton style={{width: '140px', height: '22px'}}/>
      </div>
      <MetadataPlaceholder/>
    </header>;
  }
  return <header>
    <div className={css(STYLES.titleRow)}>
      <h1 className={css(STYLES.heading)}>{props.title}</h1>
      <div className={css(STYLES.download)}>
        <Download filePath={props.filePath} disabled={false}/>
      </div>
    </div>
    <Parameters parameters={props.parameters}/>
  </header>;
}

function MetadataPlaceholder(): JSX.Element {
  const ROW_COUNT = 5;
  return <div aria-hidden className={css(STYLES.metadataPlaceholder)}>
    {Array.from({length: ROW_COUNT}, (_, index) =>
      <React.Fragment key={index}>
        <Skeleton style={{width: '80px', height: '16px'}}/>
        <Skeleton style={{width: '80px', height: '16px'}}/>
      </React.Fragment>)}
  </div>;
}

function Parameters(props: {parameters: readonly ReportDetailPage.Parameter[]}):
    JSX.Element {
  return <dl className={css(STYLES.parameters)}>
    {props.parameters.map((parameter, index) =>
      <React.Fragment key={index}>
        <dt className={css(STYLES.label)}>{parameter.label}</dt>
        <dd className={css(STYLES.value)}>{parameter.value}</dd>
      </React.Fragment>)}
  </dl>;
}

function ActionSheet(props: {filePath: string; disabled: boolean}):
    JSX.Element {
  return <section aria-label='Report Actions'
      className={css(STYLES.actionSheet)}>
    <Download {...props}/>
  </section>;
}

function Download(props: {filePath: string; disabled: boolean}): JSX.Element {
  return <div className={css(STYLES.downloadButton)}>
    <ButtonLink label='Download' href={props.filePath} download
      inert={props.disabled} style={{width: '100%'}}/>
  </div>;
}

const STYLES = StyleSheet.create({
  main: {position: 'relative', flex: '1 0 auto', boxSizing: 'border-box',
    display: 'flex', flexDirection: 'column', minWidth: 0,
    fontFamily: 'Roboto, system-ui, sans-serif', fontWeight: 400,
    fontSize: '0.875rem', color: '#333333',
    '@media (width < 768px)': {padding: '18px 18px 122px'},
    '@media (min-width: 768px)': {padding: '18px 18px 40px'}},
  titleRow: {display: 'flex', alignItems: 'center', marginBottom: '30px'},
  reportContent: {display: 'flex', flexDirection: 'column', flex: '1 0 auto',
    minWidth: 0},
  heading: {fontSize: '1.125rem', fontWeight: 500, margin: 0, minWidth: 0,
    overflowWrap: 'anywhere'},
  download: {marginLeft: 'auto', flexShrink: 0, width: '140px',
    '@media (width < 768px)': {display: 'none'}},
  downloadButton: {display: 'grid', width: '100%', maxWidth: '424px'},
  metadataPlaceholder: {display: 'grid', gridTemplateColumns: '80px 80px',
    columnGap: '58px', rowGap: '8px', width: 'fit-content'},
  parameters: {display: 'grid',
    gridTemplateColumns: '120px minmax(0, max-content)',
    columnGap: '18px', rowGap: '8px', margin: 0, width: 'fit-content',
    maxWidth: '100%', alignItems: 'flex-start'},
  label: {margin: 0, overflowWrap: 'anywhere'},
  value: {margin: 0, minWidth: 0, overflowWrap: 'anywhere'},
  content: {flex: '1 0 auto', minWidth: 0, marginTop: '30px',
    overflowX: 'auto'},
  actionSheet: {position: 'fixed', bottom: 0, left: 0, width: '100%',
    justifyContent: 'center',
    boxSizing: 'border-box', padding: '18px 18px 30px',
    backgroundColor: '#FFFFFF', boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    '@media (width < 768px)': {display: 'flex'},
    '@media (min-width: 768px)': {display: 'none'}}
});
