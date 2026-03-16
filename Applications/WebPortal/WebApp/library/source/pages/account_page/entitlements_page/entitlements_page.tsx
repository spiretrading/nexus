import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DateInput, DisplaySize, PageLayout } from '../../..';
import { EntitlementRow } from './entitlement_row';

interface Properties {

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Stores the entitlements to display. */
  entitlements: Nexus.EntitlementDatabase;

  /** The set of entitlements that are checked. */
  checked: Beam.Set<Beam.DirectoryEntry>;

  /** The comment to display. */
  comment: string;

  /** Whether the submit button is enabled. */
  canSubmit?: boolean;

  /** The effective date. */
  effectiveDate?: Beam.Date;

  /** Whether the effective date is invalid. */
  dateError?: boolean;

  /** Whether an error occurred. */
  isError?: boolean;

  /** The status message to display. */
  status?: string;

  /** The database of currencies. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The set of venues. */
  venueDatabase: Nexus.VenueDatabase;

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Indicates a change to the comment.
   * @param comment - The updated comment.
   */
  onComment?: (comment: string) => void;

  /** Indicates a change to the effective date.
   * @param date - The updated date.
   */
  onEffectiveDate?: (date?: Beam.Date) => void;

  /** Indicates an entitlement has been clicked. */
  onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void;

  /** Indicates the form should be submitted. */
  onSubmit?: () => void;
}

/** Displays a list of entitlements. */
export function EntitlementsPage(props: Properties): JSX.Element {
  return (
    <PageLayout>
      <main style={STYLE.main}>
        <Entitlements
          entitlements={props.entitlements}
          checked={props.checked}
          currencyDatabase={props.currencyDatabase}
          venueDatabase={props.venueDatabase}
          displaySize={props.displaySize}
          onEntitlementClick={props.onEntitlementClick}/>
        <EffectiveDate value={props.effectiveDate}
          error={props.dateError}
          onChange={props.onEffectiveDate}/>
        <SubmissionBlock
          comment={props.comment}
          roles={props.roles}
          canSubmit={props.canSubmit ?? false}
          isError={props.isError}
          status={props.status}
          onComment={props.onComment}
          onSubmit={props.onSubmit}/>
      </main>
    </PageLayout>);
}

function Entitlements(props: {
    entitlements: Nexus.EntitlementDatabase,
    checked: Beam.Set<Beam.DirectoryEntry>,
    currencyDatabase: Nexus.CurrencyDatabase,
    venueDatabase: Nexus.VenueDatabase,
    displaySize: DisplaySize,
    onEntitlementClick?: (entitlement: Beam.DirectoryEntry) => void
    }): JSX.Element {
  const rows = [];
  for(const entry of props.entitlements) {
    rows.push(
      <li key={entry.group.id}>
        <EntitlementRow
          entitlementEntry={entry}
          currencyEntry={props.currencyDatabase.fromCurrency(entry.currency)}
          isActive={props.checked.test(entry.group)}
          onClick={() => props.onEntitlementClick?.(entry.group)}
          displaySize={props.displaySize}
          venueDatabase={props.venueDatabase}/>
      </li>);
  }
  return (
    <section style={STYLE.sectionFirst}>
      <ul style={STYLE.entitlementList}>
        {rows}
      </ul>
    </section>);
}

function EffectiveDate(props: {
    value?: Beam.Date,
    error?: boolean,
    onChange?: (date?: Beam.Date) => void}): JSX.Element {
  return (
    <section style={STYLE.section}>
      <DateField value={props.value} error={props.error}
        onChange={props.onChange}/>
    </section>);
}

function DateField(props: {
    value?: Beam.Date,
    error?: boolean,
    onChange?: (date?: Beam.Date) => void}): JSX.Element {
  return (
    <div style={DATE_FIELD_STYLE.wrapper}>
      <DateLabel/>
      <div style={DATE_FIELD_STYLE.spacer}/>
      <div style={DATE_FIELD_STYLE.inputColumn}>
        <DateInput id='effective-date' value={props.value}
          onChange={props.onChange}
          style={DATE_FIELD_STYLE.dateInput}/>
        <DateError error={props.error}/>
      </div>
    </div>);
}

function DateLabel(): JSX.Element {
  return (
    <label htmlFor='effective-date' style={DATE_FIELD_STYLE.label}>
      Effective Date
    </label>);
}

function DateError(props: {error?: boolean}): JSX.Element {
  if(!props.error) {
    return <div style={DATE_FIELD_STYLE.errorSpacer}/>;
  }
  return (
    <div style={DATE_FIELD_STYLE.error}>
      <div style={DATE_FIELD_STYLE.errorMessageSpacer}/>
      <ErrorMessage/>
    </div>);
}

function ErrorMessage(): JSX.Element {
  return (
    <span style={DATE_FIELD_STYLE.errorMessage}>Date cannot be empty</span>);
}

function SubmissionBlock(props: {
    comment: string,
    roles: Nexus.AccountRoles,
    canSubmit: boolean,
    isError?: boolean,
    status?: string,
    onComment?: (comment: string) => void,
    onSubmit?: () => void}): JSX.Element {
  const isAdmin = props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR);
  return (
    <section style={{...STYLE.sectionLast, flexDirection: 'column',
        alignItems: 'center'}}>
      {!isAdmin &&
        <>
          <Comments value={props.comment} onChange={props.onComment}/>
          <div style={SUBMISSION_STYLE.commentsSpacer}/>
        </>}
      <Submit roles={props.roles} disabled={!props.canSubmit}
        onClick={props.onSubmit}/>
      <Feedback isError={props.isError} status={props.status}/>
    </section>);
}

function Comments(props: {
    value: string,
    onChange?: (value: string) => void}): JSX.Element {
  return (
    <textarea name='comments' style={SUBMISSION_STYLE.comments}
      value={props.value}
      placeholder='Leave comment here…'
      onChange={e => props.onChange?.(e.target.value)}/>);
}

function Submit(props: {
    roles: Nexus.AccountRoles,
    disabled: boolean,
    onClick?: () => void}): JSX.Element {
  const isAdmin = props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR);
  const label = isAdmin ? 'Save Changes' : 'Submit Request';
  return (
    <div style={SUBMISSION_STYLE.submitRow}>
      <Button label={label} disabled={props.disabled}
        onClick={props.onClick}/>
    </div>);
}

function Feedback(props: {
    isError?: boolean,
    status?: string}): JSX.Element {
  if(!props.status) {
    return <div style={SUBMISSION_STYLE.feedbackSpacer}/>;
  }
  return (
    <div style={SUBMISSION_STYLE.feedback}>
      <div style={SUBMISSION_STYLE.feedbackMessageSpacer}/>
      <FeedbackMessage isError={props.isError} status={props.status}/>
    </div>);
}

function FeedbackMessage(props: {
    isError?: boolean,
    status?: string}): JSX.Element {
  const style = props.isError ?
    SUBMISSION_STYLE.feedbackError : SUBMISSION_STYLE.feedbackSuccess;
  return <span style={style}>{props.status}</span>;
}

const STYLE: Record<string, React.CSSProperties> = {
  main: {
    padding: '18px 18px 40px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontWeight: 400,
    color: '#333333',
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    overflowY: 'auto'
  },
  entitlementList: {
    listStyle: 'none',
    margin: 0,
    padding: 0,
    containerType: 'inline-size'
  },
  sectionFirst: {
    width: '100%',
    display: 'flex',
    flexDirection: 'column',
    justifyContent: 'center',
    padding: '0 0 30px',
    borderBottom: '1px solid #E6E6E6'
  },
  section: {
    width: '100%',
    display: 'flex',
    flexDirection: 'column',
    justifyContent: 'center',
    padding: '30px 0',
    borderBottom: '1px solid #E6E6E6'
  },
  sectionLast: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center',
    padding: '30px 0 0'
  }
};

const DATE_FIELD_STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'flex-start'
  },
  label: {
    width: '96px',
    height: '34px',
    flexShrink: 0,
    display: 'flex',
    alignItems: 'center',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#333333'
  },
  spacer: {
    width: '8px',
    flexShrink: 0
  },
  inputColumn: {
    display: 'flex',
    flexDirection: 'column',
    flex: 1
  },
  dateInput: {
    maxWidth: '246px'
  },
  errorSpacer: {
    height: '26px'
  },
  error: {
    height: '26px',
    display: 'flex',
    flexDirection: 'column'
  },
  errorMessageSpacer: {
    height: '10px'
  },
  errorMessage: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    color: '#E63F44'
  }
};

const SUBMISSION_STYLE: Record<string, React.CSSProperties> = {
  comments: {
    width: '100%',
    height: '150px',
    boxSizing: 'border-box',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    padding: '10px',
    resize: 'none',
    outline: 'none'
  },
  commentsSpacer: {
    height: '30px'
  },
  submitRow: {
    display: 'flex',
    flexDirection: 'row',
    justifyContent: 'center'
  },
  feedbackSpacer: {
    height: '34px'
  },
  feedback: {
    height: '34px',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center'
  },
  feedbackMessageSpacer: {
    height: '18px'
  },
  feedbackSuccess: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    color: '#36BB55',
    textAlign: 'center'
  },
  feedbackError: {
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontSize: '0.875rem',
    color: '#E63F44',
    textAlign: 'center'
  }
};
