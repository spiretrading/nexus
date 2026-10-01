import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { IconButton, Skeleton } from '../../../components';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'children'> {}

/** Displays a loading placeholder for a scheduled report item. */
export function ScheduledReportItemPlaceholder(
    {className, ...attributes}: Properties): JSX.Element {
  return <div {...attributes} aria-hidden
      className={[css(STYLES.container), className].join(' ')}>
    <div className={css(STYLES.item)}>
      <Header/>
      <Parameters/>
      <DatePlaceholder/>
    </div>
  </div>;
}

function Header(): JSX.Element {
  return <div className={css(STYLES.header)}>
    <Skeleton style={{width: '140px', height: '16px', flexShrink: 0}}/>
    <div className={css(STYLES.headerSpace)}/>
    <IconButton icon='resources/report_page/more.svg' disabled
      aria-label='Open widget menu'
      style={{width: '24px', height: '24px', flexShrink: 0}}/>
  </div>;
}

function Parameters(): JSX.Element {
  return <div className={css(STYLES.parameters)}>
    {Array.from({length: 3}, (_, index) =>
      <div key={index} className={css(STYLES.parameter)}>
        <Skeleton style={{width: '80px', height: '16px'}}/>
        <Skeleton style={{width: '80px', height: '16px'}}/>
      </div>)}
  </div>;
}

function DatePlaceholder(): JSX.Element {
  return <div className={css(STYLES.date)}>
    <img src='resources/report_page/calendar.svg' alt='' width='12' height='12'
      className={css(STYLES.calendar)}/>
    <Skeleton style={{width: '200px', height: '16px', flexShrink: 0}}/>
  </div>;
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size',
    width: '100%'
  },
  item: {
    boxSizing: 'border-box',
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '0 17px 11px'
  },
  header: {
    display: 'flex',
    alignItems: 'center',
    marginBottom: '4px'
  },
  headerSpace: {flex: '1 1 0', marginRight: '18px'},
  parameters: {
    display: 'grid',
    rowGap: '2px',
    marginBottom: '12px',
    '@container (width < 768px)': {
      gridTemplateColumns: 'repeat(2, 80px)',
      columnGap: '66px'
    },
    '@container (min-width: 768px)': {
      gridTemplateColumns: 'repeat(3, 80px)',
      columnGap: '64px'
    }
  },
  parameter: {
    '@container (width < 768px)': {display: 'contents'},
    '@container (min-width: 768px)': {
      display: 'grid',
      rowGap: '2px'
    }
  },
  date: {
    display: 'flex',
    alignItems: 'center',
    gap: '4px'
  },
  calendar: {flex: '0 0 12px'}
});
