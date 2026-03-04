import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  date: string;
  isApproved: boolean;
  today: string;
  rdDatetime: string;
  rdTime: string;
  rdToday: string;
  category: string;
  requestState: string;
  complianceRuleStatus: string;
  entitlementStatus: string;
  diffValue: string;
  diffDirection: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    const now = new Date();
    this.state = {
      date: '2026-04-15',
      isApproved: false,
      today: now.toISOString().slice(0, 10),
      rdDatetime: now.toISOString().slice(0, 10),
      rdTime: '08:54',
      rdToday: now.toISOString().slice(0, 10),
      category: 'RISK_CONTROLS',
      requestState: 'SUBMITTED',
      complianceRuleStatus: 'ACTIVE',
      entitlementStatus: 'GRANTED',
      diffValue: '3',
      diffDirection: 'POSITIVE'
    };
  }

  public render(): JSX.Element {
    return (
      <div className={css(STYLES.panel)}>
        <h3 className={css(STYLES.heading)}>RequestEffectiveDate</h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            date
            <input className={css(STYLES.input)} type='date'
              value={this.state.date}
              onChange={e => this.setState({date: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            today
            <input className={css(STYLES.input)} type='date'
              value={this.state.today}
              onChange={e => this.setState({today: e.target.value})}/>
          </label>
          <label className={css(STYLES.checkboxLabel)}>
            <input type='checkbox'
              checked={this.state.isApproved}
              onChange={e =>
                this.setState({isApproved: e.target.checked})}/>
            isApproved
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.RequestEffectiveDate
            date={new Date(this.state.date + 'T00:00:00')}
            isApproved={this.state.isApproved}
            today={new Date(this.state.today + 'T00:00:00')}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          RelativeDate
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            datetime (date)
            <input className={css(STYLES.input)} type='date'
              value={this.state.rdDatetime}
              onChange={e => this.setState({rdDatetime: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            datetime (time)
            <input className={css(STYLES.input)} type='time'
              value={this.state.rdTime}
              onChange={e => this.setState({rdTime: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            today
            <input className={css(STYLES.input)} type='date'
              value={this.state.rdToday}
              onChange={e => this.setState({rdToday: e.target.value})}/>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.RelativeDate
            datetime={new Date(
              this.state.rdDatetime + 'T' + this.state.rdTime + ':00')}
            today={new Date(this.state.rdToday + 'T00:00:00')}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          RequestCategoryTag
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            category
            <select className={css(STYLES.input)}
              value={this.state.category}
              onChange={e => this.setState({category: e.target.value})}>
              <option value='RISK_CONTROLS'>RISK_CONTROLS</option>
              <option value='ENTITLEMENTS'>ENTITLEMENTS</option>
              <option value='COMPLIANCE'>COMPLIANCE</option>
            </select>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.RequestCategoryTag
            category={WebPortal.Category[
              this.state.category as keyof typeof WebPortal.Category]}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          RequestStateIndicator
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            state
            <select className={css(STYLES.input)}
              value={this.state.requestState}
              onChange={e =>
                this.setState({requestState: e.target.value})}>
              <option value='SUBMITTED'>SUBMITTED</option>
              <option value='MANAGER_APPROVED'>MANAGER_APPROVED</option>
              <option value='APPROVED'>APPROVED</option>
              <option value='REJECTED'>REJECTED</option>
            </select>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.RequestStateIndicator
            state={WebPortal.RequestState[
              this.state.requestState as
                keyof typeof WebPortal.RequestState]}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          ComplianceRuleStatusTag
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            status
            <select className={css(STYLES.input)}
              value={this.state.complianceRuleStatus}
              onChange={e =>
                this.setState({complianceRuleStatus: e.target.value})}>
              <option value='ACTIVE'>ACTIVE</option>
              <option value='PASSIVE'>PASSIVE</option>
              <option value='DISABLED'>DISABLED</option>
              <option value='DELETE'>DELETE</option>
              <option value='NONE'>NONE</option>
            </select>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.ComplianceRuleStatusTag
            status={WebPortal.ComplianceRuleStatusTag.Status[
              this.state.complianceRuleStatus as
                keyof typeof WebPortal.ComplianceRuleStatusTag.Status]}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          EntitlementsStatusTag
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            status
            <select className={css(STYLES.input)}
              value={this.state.entitlementStatus}
              onChange={e =>
                this.setState({entitlementStatus: e.target.value})}>
              <option value='GRANTED'>GRANTED</option>
              <option value='REVOKED'>REVOKED</option>
            </select>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.EntitlementsStatusTag
            status={WebPortal.EntitlementsStatusTag.Status[
              this.state.entitlementStatus as
                keyof typeof WebPortal.EntitlementsStatusTag.Status]}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          DiffBadge
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            value
            <input className={css(STYLES.input)} type='text'
              value={this.state.diffValue}
              onChange={e => this.setState({diffValue: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            direction
            <select className={css(STYLES.input)}
              value={this.state.diffDirection}
              onChange={e =>
                this.setState({diffDirection: e.target.value})}>
              <option value='POSITIVE'>POSITIVE</option>
              <option value='NEGATIVE'>NEGATIVE</option>
              <option value='NONE'>NONE</option>
            </select>
          </label>
        </div>
        <div className={css(STYLES.preview)}>
          <WebPortal.DiffBadge
            value={this.state.diffValue}
            direction={WebPortal.DiffBadge.Direction[
              this.state.diffDirection as
                keyof typeof WebPortal.DiffBadge.Direction]}/>
        </div>
      </div>);
  }
}

const STYLES = StyleSheet.create({
  panel: {
    padding: '20px',
    fontFamily: 'Roboto'
  },
  controls: {
    display: 'flex',
    gap: '16px',
    alignItems: 'flex-end' as 'flex-end',
    marginBottom: '16px',
    flexWrap: 'wrap' as 'wrap'
  },
  label: {
    display: 'flex',
    flexDirection: 'column' as 'column',
    fontSize: '12px',
    color: '#666',
    gap: '4px'
  },
  checkboxLabel: {
    display: 'flex',
    alignItems: 'center' as 'center',
    fontSize: '12px',
    color: '#666',
    gap: '4px'
  },
  heading: {
    margin: '0 0 12px 0',
    fontSize: '16px'
  },
  separator: {
    marginTop: '24px'
  },
  input: {
    padding: '4px 8px',
    fontSize: '14px'
  },
  preview: {
    padding: '12px',
    border: '1px dashed #CCC'
  }
});

ReactDOM.render(<App/>, document.getElementById('main'));
