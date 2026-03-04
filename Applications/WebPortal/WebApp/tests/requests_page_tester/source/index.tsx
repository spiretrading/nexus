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
  rccName: string;
  rccOldValue: string;
  rccNewValue: string;
  rccDeltaValue: string;
  rccDeltaDirection: string;
  widthRedDate: number;
  widthRelDate: number;
  widthCatTag: number;
  widthStateInd: number;
  widthCompTag: number;
  widthEntTag: number;
  widthDiffBadge: number;
  widthRcc: number;
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
      diffDirection: 'POSITIVE',
      rccName: 'Buying Power',
      rccOldValue: '$100,000',
      rccNewValue: '$150,000',
      rccDeltaValue: '$50,000',
      rccDeltaDirection: 'POSITIVE',
      widthRedDate: 400,
      widthRelDate: 400,
      widthCatTag: 400,
      widthStateInd: 400,
      widthCompTag: 400,
      widthEntTag: 400,
      widthDiffBadge: 400,
      widthRcc: 800
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthRedDate}px
          <input type='range' min='0' max='1200'
            value={this.state.widthRedDate}
            onChange={e =>
              this.setState({widthRedDate: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthRedDate + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthRelDate}px
          <input type='range' min='0' max='1200'
            value={this.state.widthRelDate}
            onChange={e =>
              this.setState({widthRelDate: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthRelDate + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthCatTag}px
          <input type='range' min='0' max='1200'
            value={this.state.widthCatTag}
            onChange={e =>
              this.setState({widthCatTag: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthCatTag + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthStateInd}px
          <input type='range' min='0' max='1200'
            value={this.state.widthStateInd}
            onChange={e =>
              this.setState({widthStateInd: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthStateInd + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthCompTag}px
          <input type='range' min='0' max='1200'
            value={this.state.widthCompTag}
            onChange={e =>
              this.setState({widthCompTag: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthCompTag + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthEntTag}px
          <input type='range' min='0' max='1200'
            value={this.state.widthEntTag}
            onChange={e =>
              this.setState({widthEntTag: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthEntTag + 'px'}}>
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
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthDiffBadge}px
          <input type='range' min='0' max='1200'
            value={this.state.widthDiffBadge}
            onChange={e =>
              this.setState({widthDiffBadge: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthDiffBadge + 'px'}}>
          <WebPortal.DiffBadge
            value={this.state.diffValue}
            direction={WebPortal.DiffBadge.Direction[
              this.state.diffDirection as
                keyof typeof WebPortal.DiffBadge.Direction]}/>
        </div>
        <h3 className={css(STYLES.heading, STYLES.separator)}>
          RiskControlsChangeItem
        </h3>
        <div className={css(STYLES.controls)}>
          <label className={css(STYLES.label)}>
            name
            <input className={css(STYLES.input)} type='text'
              value={this.state.rccName}
              onChange={e => this.setState({rccName: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            oldValue
            <input className={css(STYLES.input)} type='text'
              value={this.state.rccOldValue}
              onChange={e =>
                this.setState({rccOldValue: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            newValue
            <input className={css(STYLES.input)} type='text'
              value={this.state.rccNewValue}
              onChange={e =>
                this.setState({rccNewValue: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            delta.value
            <input className={css(STYLES.input)} type='text'
              value={this.state.rccDeltaValue}
              onChange={e =>
                this.setState({rccDeltaValue: e.target.value})}/>
          </label>
          <label className={css(STYLES.label)}>
            delta.direction
            <select className={css(STYLES.input)}
              value={this.state.rccDeltaDirection}
              onChange={e =>
                this.setState({rccDeltaDirection: e.target.value})}>
              <option value='POSITIVE'>POSITIVE</option>
              <option value='NEGATIVE'>NEGATIVE</option>
              <option value='NONE'>NONE</option>
            </select>
          </label>
        </div>
        <label className={css(STYLES.sliderLabel)}>
          width: {this.state.widthRcc}px
          <input type='range' min='0' max='1200'
            value={this.state.widthRcc}
            onChange={e =>
              this.setState({widthRcc: parseInt(e.target.value)})}/>
        </label>
        <div className={css(STYLES.preview)}
          style={{width: this.state.widthRcc + 'px'}}>
          <WebPortal.RiskControlsChangeItem
            name={this.state.rccName}
            oldValue={this.state.rccOldValue}
            newValue={this.state.rccNewValue}
            delta={{
              value: this.state.rccDeltaValue,
              direction: WebPortal.DiffBadge.Direction[
                this.state.rccDeltaDirection as
                  keyof typeof WebPortal.DiffBadge.Direction]
            }}/>
        </div>
      </div>);
  }
}

const STYLES = StyleSheet.create({
  panel: {
    padding: '20px',
    fontFamily: 'Roboto',
    overflowY: 'auto' as 'auto',
    height: '100vh',
    boxSizing: 'border-box' as 'border-box'
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
  sliderLabel: {
    display: 'flex',
    alignItems: 'center' as 'center',
    fontSize: '12px',
    color: '#666',
    gap: '8px',
    marginBottom: '8px'
  },
  preview: {
    border: '1px dashed #CCC'
  }
});

ReactDOM.render(<App/>, document.getElementById('main'));
