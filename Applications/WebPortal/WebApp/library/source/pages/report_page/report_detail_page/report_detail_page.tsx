import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { ButtonLink, PageLayout } from '../../../components';

interface Properties {

  /** The report heading. */
  title: string;

  /** The report parameters in display order. */
  parameters: readonly ReportDetailPage.Parameter[];

  /** The URL from which the generated file can be downloaded. */
  filePath: string;

  /** The report viewer supplied by the host application. */
  content?: React.ReactNode;
}

/** Displays a generated report, its parameters, and its download link. */
export function ReportDetailPage(props: Properties): JSX.Element {
  return <PageLayout>
    <Main {...props}/>
  </PageLayout>;
}

export namespace ReportDetailPage {

  /** A report parameter and its formatted display value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }
}

function Main(props: Properties): JSX.Element {
  return <main className={css(STYLES.main)}>
    <Header {...props}/>
    <div className={css(STYLES.content)}>{props.content}</div>
    <ActionSheet filePath={props.filePath}/>
  </main>;
}

function Header(props: Properties): JSX.Element {
  return <header>
    <div className={css(STYLES.titleRow)}>
      <h1 className={css(STYLES.heading)}>{props.title}</h1>
      <div className={css(STYLES.download)}>
        <ButtonLink label='Download' href={props.filePath} download
          style={{width: '140px'}}/>
      </div>
    </div>
    <Parameters parameters={props.parameters}/>
  </header>;
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

function ActionSheet(props: {filePath: string}): JSX.Element {
  return <section aria-label='Report Actions'
      className={css(STYLES.actionSheet)}>
    <ButtonLink label='Download' href={props.filePath} download
      style={{width: '100%'}}/>
  </section>;
}

const STYLES = StyleSheet.create({
  main: {position: 'relative', flex: '1 0 auto', boxSizing: 'border-box',
    display: 'flex', flexDirection: 'column', minWidth: 0,
    fontFamily: 'Roboto, system-ui, sans-serif', fontWeight: 400,
    color: '#333333',
    '@media (width < 768px)': {padding: '18px 18px 122px'},
    '@media (min-width: 768px)': {padding: '18px 18px 40px'}},
  titleRow: {display: 'flex', alignItems: 'center', marginBottom: '30px'},
  heading: {fontSize: '1.125rem', fontWeight: 500, margin: 0, minWidth: 0,
    overflowWrap: 'anywhere'},
  download: {marginLeft: 'auto', flexShrink: 0,
    '@media (width < 768px)': {display: 'none'}},
  parameters: {display: 'grid',
    gridTemplateColumns: '120px minmax(0, max-content)',
    columnGap: '18px', rowGap: '8px', margin: 0, width: 'fit-content',
    maxWidth: '100%', alignItems: 'center'},
  label: {margin: 0, overflowWrap: 'anywhere'},
  value: {margin: 0, minWidth: 0, overflowWrap: 'anywhere'},
  content: {flex: '1 0 auto', minWidth: 0, marginTop: '30px',
    overflowX: 'auto'},
  actionSheet: {position: 'absolute', bottom: 0, left: 0, width: '100%',
    boxSizing: 'border-box', padding: '18px 18px 30px',
    backgroundColor: '#FFFFFF', boxShadow: '0 0 6px rgb(0 0 0 / 25%)',
    '@media (min-width: 768px)': {display: 'none'}}
});
