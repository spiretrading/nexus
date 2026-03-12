import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DateField as DateInput, DisplaySize, HLine, PageLayout } from '../../..';
import { SubmissionInput } from '..';
import { RiskParametersView } from './risk_parameters_view';

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
  onEffectiveDate?: (date: Beam.Date) => void;

  /**
   * Indicates the form should be submitted.
   * @param comment - The comment to submit with the form.
   * @param parameters - The parameters to submit.
   */
  onSubmit?: (comment: string, parameters: Nexus.RiskParameters) => void;
}

/** Displays a risk page. */
export class RiskPage extends React.Component<Properties> {
  public render(): JSX.Element {
    return(
      <PageLayout>
        <div style={RiskPage.STYLE.content}>
          <RiskParametersView
            parameters={this.props.parameters}
            currencyDatabase={this.props.currencyDatabase}
            onChange={this.props.onParameters}/>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <div style={RiskPage.STYLE.lineWrapper}>
            <HLine color={RiskPage.LINE_COLOR}/>
          </div>
          <div style={RiskPage.STYLE.dateFieldPadding}/>
          <DateField value={this.props.effectiveDate}
            onChange={this.props.onEffectiveDate}/>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <div style={RiskPage.STYLE.lineWrapper}>
            <HLine color={RiskPage.LINE_COLOR}/>
          </div>
          <div style={RiskPage.STYLE.mediumPadding}/>
          <SubmissionInput comment={this.props.comment}
            roles={this.props.roles} isError={this.props.isError}
            status={this.props.status ?? ''}
            isEnabled={this.props.canSubmit ?? false}
            onChange={this.props.onComment}
            onSubmit={this.onSubmit}/>
        </div>
      </PageLayout>);
  }

  private onSubmit = () => {
    this.props.onSubmit?.(this.props.comment, this.props.parameters.clone());
  }

  private static readonly STYLE = {
    content: {
      paddingTop: '30px',
      paddingBottom: '40px',
      paddingLeft: '18px',
      paddingRight: '18px',
      boxSizing: 'border-box',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center'
    } as React.CSSProperties,
    mediumPadding: {
      width: '100%',
      height: '30px'
    } as React.CSSProperties,
    dateFieldPadding: {
      width: '100%',
      height: '32px'
    } as React.CSSProperties,
    lineWrapper: {
      width: '100%'
    } as React.CSSProperties
  };
  private static readonly LINE_COLOR = '#E6E6E6';
}

function today(): Beam.Date {
  const now = new Date();
  return new Beam.Date(now.getFullYear(), now.getMonth() + 1, now.getDate());
}

function DateField(props: {
    value?: Beam.Date,
    onChange?: (date: Beam.Date) => void}): JSX.Element {
  return (
    <div style={DATE_FIELD_STYLE.wrapper}>
      <label htmlFor='effective-date' style={DATE_FIELD_STYLE.label}>
        Effective Date
      </label>
      <div style={DATE_FIELD_STYLE.labelPadding}/>
      <DateInput displaySize={DisplaySize.SMALL}
        value={props.value ?? today()} onChange={props.onChange}/>
      <div style={DATE_FIELD_STYLE.errorSpace}/>
    </div>);
}

const DATE_FIELD_STYLE = {
  wrapper: {
    width: '246px',
    display: 'flex',
    flexDirection: 'column'
  } as React.CSSProperties,
  label: {
    font: '400 14px Roboto',
    color: '#333333'
  } as React.CSSProperties,
  labelPadding: {
    height: '12px'
  } as React.CSSProperties,
  errorSpace: {
    flex: '1 1 auto'
  } as React.CSSProperties
};
