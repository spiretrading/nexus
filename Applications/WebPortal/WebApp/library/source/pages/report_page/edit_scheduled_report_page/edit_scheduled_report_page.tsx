import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { PageLayout } from '../../../components';
import { ReportFormTemplate } from '../report_form_template';

interface Properties extends
  Omit<React.ComponentProps<typeof ReportFormTemplate>,
    'title' | 'mode' | 'showRuntime'> {}

/** Displays the form used to edit an existing scheduled report. */
export function EditScheduledReportPage(props: Properties): JSX.Element {
  return <PageLayout>
    <Main {...props}/>
  </PageLayout>;
}

function Main(props: Properties): JSX.Element {
  return <main className={css(STYLES.main)}>
    <ReportFormTemplate {...props} title='Edit Scheduled Report'
      mode={ReportFormTemplate.Mode.EDIT} showRuntime={false}
      value={{...props.value, scheduled: true}}/>
  </main>;
}

const STYLES = StyleSheet.create({
  main: {boxSizing: 'border-box', padding: '18px 18px 40px',
    fontFamily: 'Roboto, system-ui, sans-serif', fontWeight: 400,
    color: '#333333'}
});
