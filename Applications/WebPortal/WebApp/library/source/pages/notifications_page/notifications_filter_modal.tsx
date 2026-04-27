import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DateInput, LabeledCheckbox, Modal } from '../../components';
import { NotificationsFilter } from './notifications_model';

interface Properties {

  /** The current filter state. */
  filter: NotificationsFilter;

  /** Called when the filter is submitted. */
  onSubmit?: (filter: NotificationsFilter) => void;

  /** Called when the modal is closed. */
  onClose?: () => void;
}

interface State {
  categories: Set<Nexus.Notification.Category>;
  startDate: Beam.Date;
  endDate: Beam.Date;
}

/** A modal for filtering notifications by category and date range. */
export class NotificationsFilterModal
    extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      categories: props.filter.categories,
      startDate: props.filter.startDate,
      endDate: props.filter.endDate
    };
  }

  public render(): JSX.Element {
    const isInvalid = this.state.startDate && this.state.endDate &&
      this.state.startDate.toDate() > this.state.endDate.toDate();
    return (
      <Modal title='Filters' onClose={this.props.onClose}>
        <div className={css(STYLES.content)}>
          <section className={css(STYLES.filters)}>
            <fieldset className={css(STYLES.fieldset)}>
              <legend className={css(STYLES.legend)}>
                Categories
              </legend>
              <div className={css(STYLES.fieldGap)}/>
              <LabeledCheckbox label='Account Modification'
                isChecked={this.state.categories.has(
                  Nexus.Notification.Category.ACCOUNT_MODIFICATION)}
                onChange={(isChecked) => this.onCategoryChange(
                  Nexus.Notification.Category.ACCOUNT_MODIFICATION, isChecked)}/>
              <LabeledCheckbox label='Report'
                isChecked={this.state.categories.has(
                  Nexus.Notification.Category.REPORT)}
                onChange={(isChecked) => this.onCategoryChange(
                  Nexus.Notification.Category.REPORT, isChecked)}/>
            </fieldset>
            <div className={css(STYLES.sectionGap)}/>
            <fieldset className={css(STYLES.fieldset)}>
              <legend className={css(STYLES.legend)}>
                Date Created
              </legend>
              <div className={css(STYLES.fieldGap)}/>
              <div className={css(STYLES.dateRow)}>
                <label htmlFor='start-date' className={css(STYLES.dateLabel)}>
                  Start
                </label>
                <div className={css(STYLES.dateSpacer)}/>
                <DateInput id='start-date'
                  value={this.state.startDate}
                  onChange={this.onStartDateChange}
                  className={css(STYLES.dateInput)}/>
              </div>
              <div className={css(STYLES.dateRow)}>
                <label htmlFor='end-date' className={css(STYLES.dateLabel)}>
                  End
                </label>
                <div className={css(STYLES.dateSpacer)}/>
                <DateInput id='end-date'
                  value={this.state.endDate}
                  onChange={this.onEndDateChange}
                  className={css(STYLES.dateInput)}/>
              </div>
            </fieldset>
            <div className={css(STYLES.errorSection)}>
              {isInvalid &&
                <span className={css(STYLES.errorMessage)}>
                  End date must be greater than start date
                </span>}
            </div>
          </section>
          <section className={css(STYLES.actions)}>
            <Button label='Apply' onClick={this.onSubmit}
              disabled={isInvalid}
              style={{width: '100%'}}/>
          </section>
        </div>
      </Modal>);
  }

  private onCategoryChange = (
      category: Nexus.Notification.Category, isChecked: boolean) => {
    const updated = new Set(this.state.categories);
    if(isChecked) {
      updated.add(category);
    } else {
      updated.delete(category);
    }
    this.setState({categories: updated});
  };

  private onStartDateChange = (date: Beam.Date) => {
    this.setState({startDate: date});
  };

  private onEndDateChange = (date: Beam.Date) => {
    this.setState({endDate: date});
  };

  private onSubmit = () => {
    this.props.onSubmit?.({
      query: this.props.filter.query,
      categories: this.state.categories,
      startDate: this.state.startDate,
      endDate: this.state.endDate
    });
  };
}

const STYLES = StyleSheet.create({
  content: {
    backgroundColor: '#FFFFFF',
    width: '282px',
    height: '568px',
    overflowY: 'auto',
    position: 'relative' as 'relative'
  },
  filters: {
    padding: '30px 18px 100px'
  },
  fieldset: {
    display: 'flex',
    flexDirection: 'column',
    gap: '10px',
    border: 'none',
    padding: 0,
    margin: 0
  },
  legend: {
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#333333',
    margin: 0,
    padding: 0
  },
  fieldGap: {
    height: '8px'
  },
  sectionGap: {
    height: '30px'
  },
  dateRow: {
    display: 'flex',
    alignItems: 'center'
  },
  dateLabel: {
    fontSize: '0.875rem',
    color: '#333333',
    margin: 0,
    width: '3.375rem',
    flexShrink: 0
  },
  dateSpacer: {
    width: '8px',
    flexShrink: 0
  },
  dateInput: {
    flex: 1
  },
  errorSection: {
    padding: '0 0 18px',
    minHeight: '50px'
  },
  errorMessage: {
    display: 'block',
    paddingTop: '18px',
    fontSize: '0.875rem',
    color: '#E63F44'
  },
  actions: {
    position: 'absolute' as 'absolute',
    inset: 'auto 0 0',
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    borderTop: '1px solid #E6E6E6'
  }
});
