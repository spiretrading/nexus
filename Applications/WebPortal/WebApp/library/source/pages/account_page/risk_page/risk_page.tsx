import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, CurrencySelect, DateInput, DurationInput, MoneyInput,
  PageLayout } from '../../..';

interface Properties {

  /** Used to lookup currency names and symbols. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The comment to include in the submission. */
  comment: string;

  /** The risk parameters to display. */
  parameters: Nexus.RiskParameters;

  /** The effective date. */
  effectiveDate?: Beam.Date;

  /** The account's roles. */
  roles: Nexus.AccountRoles;

  /** Whether the changes can be submitted. */
  canSubmit?: boolean;

  /** Whether an error occurred. */
  isError?: boolean;

  /** Whether the effective date is invalid. */
  dateError?: boolean;

  /** The status message to display. */
  status?: string;

  /**
   * Indicates a change to the comment.
   * @param comment The updated comment.
   */
  onComment?: (comment: string) => void;

  /**
   * Indicates a change to the risk parameters.
   * @param parameters The updated parameters.
   */
  onParameters?: (parameters: Nexus.RiskParameters) => void;

  /**
   * Indicates a change to the effective date.
   * @param date - The updated date.
   */
  onEffectiveDate?: (date?: Beam.Date) => void;

  /**
   * Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   * @param parameters - The parameters to submit.
   * @param effectiveDate - The effective date to submit.
   */
  onSubmit?: (comment: string, parameters: Nexus.RiskParameters,
    effectiveDate: Beam.Date) => void;
}

/** Displays a risk page. */
export function RiskPage(props: Properties): JSX.Element {
  const onSubmit = () => {
    props.onSubmit?.(props.comment, props.parameters.clone(),
      props.effectiveDate ?? Beam.Date.today());
  };
  return (
    <PageLayout>
      <main style={STYLE.main}>
        <RiskControls
          parameters={props.parameters}
          currencyDatabase={props.currencyDatabase}
          onChange={props.onParameters}/>
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
          onSubmit={onSubmit}/>
      </main>
    </PageLayout>);
}

function RiskControls(props: {
    parameters: Nexus.RiskParameters,
    currencyDatabase: Nexus.CurrencyDatabase,
    onChange?: (parameters: Nexus.RiskParameters) => void}): JSX.Element {
  return (
    <section style={STYLE.sectionFirst}>
      <Controls parameters={props.parameters}
        currencyDatabase={props.currencyDatabase}
        onChange={props.onChange}/>
    </section>);
}

function Controls(props: {
    parameters: Nexus.RiskParameters,
    currencyDatabase: Nexus.CurrencyDatabase,
    onChange?: (parameters: Nexus.RiskParameters) => void}): JSX.Element {
  const currencySign = props.currencyDatabase.fromCurrency(
    props.parameters.currency).sign;
  const onCurrencyChange = (value: Nexus.Currency) => {
    const p = props.parameters.clone();
    p.currency = value;
    props.onChange?.(p);
  };
  const onBuyingPowerChange = (value?: Nexus.Money) => {
    if(value == null) return;
    const p = props.parameters.clone();
    p.buyingPower = value;
    props.onChange?.(p);
  };
  const onNetLossChange = (value?: Nexus.Money) => {
    if(value == null) return;
    const p = props.parameters.clone();
    p.netLoss = value;
    props.onChange?.(p);
  };
  const onTransitionTimeChange = (value?: Beam.Duration) => {
    if(value == null) return;
    const p = props.parameters.clone();
    p.transitionTime = value;
    props.onChange?.(p);
  };
  return (
    <div style={CONTROLS_STYLE.wrapper}>
      <CurrencyLabel/>
      <div style={CONTROLS_STYLE.labelSpacer}/>
      <Currency currencyDatabase={props.currencyDatabase}
        value={props.parameters.currency} onChange={onCurrencyChange}/>
      <div style={CONTROLS_STYLE.fieldSpacer}/>
      <BuyingPowerLabel sign={currencySign}/>
      <div style={CONTROLS_STYLE.labelSpacer}/>
      <BuyingPower value={props.parameters.buyingPower}
        onChange={onBuyingPowerChange}/>
      <div style={CONTROLS_STYLE.fieldSpacer}/>
      <NetLossLabel sign={currencySign}/>
      <div style={CONTROLS_STYLE.labelSpacer}/>
      <NetLoss value={props.parameters.netLoss}
        onChange={onNetLossChange}/>
      <div style={CONTROLS_STYLE.fieldSpacer}/>
      <TransitionTimeLabel/>
      <div style={CONTROLS_STYLE.labelSpacer}/>
      <TransitionTime value={props.parameters.transitionTime}
        onChange={onTransitionTimeChange}/>
    </div>);
}

function CurrencyLabel(): JSX.Element {
  return (
    <label htmlFor='currency' style={CONTROLS_STYLE.label}>
      Currency
    </label>);
}

function Currency(props: {
    currencyDatabase: Nexus.CurrencyDatabase,
    value: Nexus.Currency,
    onChange?: (value: Nexus.Currency) => void}): JSX.Element {
  return (
    <CurrencySelect id='currency' name='currency'
      style={{width: '100%'}}
      currencyDatabase={props.currencyDatabase}
      value={props.value} onChange={props.onChange}/>);
}

function BuyingPowerLabel(props: {sign: string}): JSX.Element {
  return (
    <label htmlFor='buying-power' style={CONTROLS_STYLE.label}>
      {`Buying Power (${props.sign})`}
    </label>);
}

function BuyingPower(props: {
    value: Nexus.Money,
    onChange?: (value?: Nexus.Money) => void}): JSX.Element {
  return (
    <MoneyInput id='buying-power' name='buying-power'
      value={props.value} min={Nexus.Money.ZERO}
      onChange={props.onChange}/>);
}

function NetLossLabel(props: {sign: string}): JSX.Element {
  return (
    <label htmlFor='net-loss' style={CONTROLS_STYLE.label}>
      {`Net Loss (${props.sign})`}
    </label>);
}

function NetLoss(props: {
    value: Nexus.Money,
    onChange?: (value?: Nexus.Money) => void}): JSX.Element {
  return (
    <MoneyInput id='net-loss' name='net-loss'
      value={props.value} min={Nexus.Money.ZERO}
      onChange={props.onChange}/>);
}

function TransitionTimeLabel(): JSX.Element {
  return (
    <label htmlFor='transition-time' style={CONTROLS_STYLE.label}>
      Transition Time
    </label>);
}

function TransitionTime(props: {
    value: Beam.Duration,
    onChange?: (value?: Beam.Duration) => void}): JSX.Element {
  return (
    <DurationInput value={props.value} onChange={props.onChange}/>);
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
      <DateInput id='effective-date' value={props.value}
        onChange={props.onChange}/>
      <DateError error={props.error}/>
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
  const label = props.roles.test(Nexus.AccountRoles.Role.ADMINISTRATOR) ?
    'Save Changes' : 'Submit Request';
  return (
    <div style={SUBMISSION_STYLE.submitRow}>
      <Button label={label} disabled={props.disabled}
        style={SUBMISSION_STYLE.submitButton} onClick={props.onClick}/>
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
      <FeedbackMessage isError={props.isError}/>
    </div>);
}

function FeedbackMessage(props: {isError?: boolean}): JSX.Element {
  const message = props.isError ? 'Server issue' : 'Saved';
  const color = props.isError ? '#E63F44' : '#36BB55';
  return (
    <span style={{...SUBMISSION_STYLE.feedbackMessage, color}}>
      {message}
    </span>);
}

const STYLE: Record<string, React.CSSProperties> = {
  main: {
    padding: '30px 18px 40px',
    fontFamily: "'Roboto', system-ui, sans-serif",
    fontWeight: 400,
    color: '#333333',
    boxSizing: 'border-box',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    overflowY: 'auto'
  },
  section: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center',
    padding: '30px 0',
    borderBottom: '1px solid #E6E6E6'
  },
  sectionFirst: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center',
    paddingTop: 0,
    paddingBottom: '30px',
    borderBottom: '1px solid #E6E6E6'
  },
  sectionLast: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center',
    paddingTop: '30px',
    paddingBottom: 0
  }
};

const CONTROLS_STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '246px',
    display: 'flex',
    flexDirection: 'column'
  },
  label: {
    fontSize: '0.875rem'
  },
  labelSpacer: {
    height: '12px'
  },
  fieldSpacer: {
    height: '32px'
  }
};

const DATE_FIELD_STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    width: '246px',
    display: 'flex',
    flexDirection: 'column'
  },
  label: {
    fontSize: '0.875rem'
  },
  spacer: {
    height: '12px'
  },
  errorSpacer: {
    height: '26px'
  },
  error: {
    display: 'flex',
    flexDirection: 'column'
  },
  errorMessageSpacer: {
    height: '10px'
  },
  errorMessage: {
    fontSize: '0.875rem',
    color: '#E63F44'
  }
};

const SUBMISSION_STYLE: Record<string, React.CSSProperties> = {
  comments: {
    width: '100%',
    height: '150px',
    boxSizing: 'border-box',
    resize: 'none',
    fontFamily: 'inherit',
    fontSize: '0.875rem',
    fontWeight: 'inherit',
    color: 'inherit',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    padding: '3px 9px',
    outline: 'none'
  },
  commentsSpacer: {
    height: '30px'
  },
  submitRow: {
    width: '100%',
    display: 'flex',
    justifyContent: 'center'
  },
  submitButton: {
    width: '246px'
  },
  feedback: {
    width: '100%',
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center'
  },
  feedbackSpacer: {
    height: '34px'
  },
  feedbackMessageSpacer: {
    height: '20px'
  },
  feedbackMessage: {
    fontSize: '0.875rem',
    textAlign: 'center'
  }
};
