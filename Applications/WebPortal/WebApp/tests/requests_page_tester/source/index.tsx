import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface State {
  date: string;
  isApproved: boolean;
  today: string;
}

class App extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      date: '2026-04-15',
      isApproved: false,
      today: new Date().toISOString().slice(0, 10)
    };
  }

  public render(): JSX.Element {
    return (
      <div className={css(STYLES.panel)}>
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
