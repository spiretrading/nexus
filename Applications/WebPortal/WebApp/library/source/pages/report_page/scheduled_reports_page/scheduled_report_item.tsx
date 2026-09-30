import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { IconButton } from '../../../components';
import { highlightText } from './highlight_text';
import { ScheduledDate } from './scheduled_date';
import { ScheduledReportItemContextMenu } from
  './scheduled_report_item_context_menu';

interface Properties extends
    Omit<React.HTMLAttributes<HTMLDivElement>, 'id' | 'children'> {

  /** The identifier of the scheduled report. */
  id: string;

  /** The report type displayed in the heading. */
  type: string;

  /** The report parameters, in display order. */
  parameters: readonly ScheduledReportItem.Parameter[];

  /** Whether the report runs repeatedly. Defaults to false. */
  repeats?: boolean;

  /** The next run date, or the run date of a one-time report. */
  runDate: ScheduledDate.Date;

  /** Literal text to highlight in the report's content, ignoring case. */
  highlight?: string;

  /** Called when the user requests an immediate run. */
  onRun?: () => void;

  /** Called when the user requests a duplicate scheduled report. */
  onDuplicate?: () => void;

  /** Called when the user requests deletion of the scheduled report. */
  onDelete?: () => void;
}

/** Displays a scheduled report with its edit link and action menu. */
export class ScheduledReportItem extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.identifier = `scheduled-report-${ScheduledReportItem.nextId++}`;
    this.invoker = React.createRef();
  }

  public render(): JSX.Element {
    const {id, type, parameters, repeats, runDate, onRun, onDuplicate,
      onDelete, highlight, className, ...attributes} = this.props;
    const title = (() => {
      if(repeats) {
        return `Recurring ${type}`;
      }
      return type;
    })();
    return <div {...attributes}
        className={[css(STYLES.container), className].join(' ')}>
      <div className={css(STYLES.item)}>
        <a href={`/reports/edit/${encodeURIComponent(id)}`}
          aria-labelledby={`${this.identifier}-title`}
          className={css(STYLES.link)}/>
        <Header title={title} highlight={highlight} identifier={this.identifier}
          invoker={this.invoker} onSubmit={this.onCommand}/>
        <Parameters parameters={parameters} highlight={highlight}/>
        <ScheduledDate date={runDate} repeats={repeats} highlight={highlight}/>
      </div>
    </div>;
  }

  private onCommand = (command: ScheduledReportItemContextMenu.Command) => {
    if(command === 'run') {
      this.props.onRun?.();
    } else if(command === 'duplicate') {
      this.props.onDuplicate?.();
    } else if(command === 'delete') {
      this.props.onDelete?.();
    }
  };

  private static nextId = 0;
  private identifier: string;
  private invoker: React.RefObject<HTMLButtonElement>;
}

export namespace ScheduledReportItem {

  /** A report parameter and its formatted value. */
  export interface Parameter {

    /** The parameter label. */
    label: string;

    /** The formatted parameter value. */
    value: string;
  }
}

interface HeaderProperties {
  title: string;
  highlight: string;
  identifier: string;
  invoker: React.RefObject<HTMLButtonElement>;
  onSubmit: (command: ScheduledReportItemContextMenu.Command) => void;
}

function Header(props: HeaderProperties): JSX.Element {
  const menuId = `${props.identifier}-menu`;
  const anchor = `--${menuId}`;
  return <header className={css(STYLES.header)}>
    <h2 id={`${props.identifier}-title`} className={css(STYLES.title)}>
      {highlightText(props.title, props.highlight)}
    </h2>
    <span className={css(STYLES.menuContainer)}>
      <IconButton icon='resources/report_page/more.svg'
        buttonRef={props.invoker} aria-label='Open widget menu'
        {...{popovertarget: menuId}}
        style={{width: '24px', height: '24px', anchorName: anchor} as
          React.CSSProperties}/>
      <ScheduledReportItemContextMenu id={menuId} invoker={props.invoker}
        className={css(STYLES.menu)}
        style={{positionAnchor: anchor} as React.CSSProperties}
        onSubmit={props.onSubmit}/>
    </span>
  </header>;
}

interface ParametersProperties {
  parameters: readonly ScheduledReportItem.Parameter[];
  highlight: string;
}

function Parameters(props: ParametersProperties): JSX.Element {
  return <dl className={css(STYLES.parameters)}>
    {props.parameters.map((parameter, index) =>
      <div key={index} className={css(STYLES.parameter)}>
        <dt className={css(STYLES.label)}>
          {highlightText(parameter.label, props.highlight)}
        </dt>
        <dd className={css(STYLES.value)}>
          {highlightText(parameter.value, props.highlight)}
        </dd>
      </div>)}
  </dl>;
}

const STYLES = StyleSheet.create({
  container: {
    containerType: 'inline-size',
    width: '100%'
  },
  item: {
    position: 'relative',
    boxSizing: 'border-box',
    border: '1px solid transparent',
    borderBottomColor: '#E6E6E6',
    backgroundColor: '#FFFFFF',
    padding: '0 17px 11px',
    fontFamily: 'Roboto, system-ui, sans-serif',
    fontWeight: 400,
    fontSize: '0.875rem',
    color: '#333333',
    ':hover': {backgroundColor: '#F8F8F8'},
    ':has(> a:focus-visible)': {borderColor: '#684BC7'}
  },
  link: {
    position: 'absolute',
    inset: 0,
    zIndex: 0,
    outline: 'none'
  },
  header: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    gap: '18px',
    padding: '3px 0 4px',
    marginBottom: '4px'
  },
  title: {
    margin: 0,
    fontSize: 'inherit',
    fontWeight: 500,
    overflowWrap: 'anywhere'
  },
  menuContainer: {
    display: 'inline-flex',
    position: 'relative',
    zIndex: 1,
    flexShrink: 0,
    ':has(> [popover]:popover-open) > button': {
      backgroundColor: '#F8F8F8',
      color: '#4B23A0'
    }
  },
  menu: {
    positionArea: 'bottom span-right',
    positionTryFallbacks: 'flip-block flip-inline',
    margin: 0
  },
  parameters: {
    margin: '0 0 12px',
    '@container (width < 768px)': {
      display: 'grid',
      gridTemplateColumns: '138px minmax(0, 1fr)',
      columnGap: '8px',
      rowGap: '2px'
    },
    '@container (min-width: 768px)': {
      display: 'flex',
      gap: '8px'
    }
  },
  parameter: {
    '@container (width < 768px)': {display: 'contents'},
    '@container (min-width: 768px)': {
      display: 'grid',
      gridTemplateColumns: '136px',
      alignContent: 'start',
      rowGap: '2px',
      flex: '0 0 136px'
    }
  },
  label: {fontWeight: 500, color: '#5D5E6D', overflowWrap: 'anywhere'},
  value: {margin: 0, overflowWrap: 'anywhere'}
});
