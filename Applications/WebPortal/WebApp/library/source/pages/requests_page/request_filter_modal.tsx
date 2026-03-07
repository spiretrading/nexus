import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button } from '../../components/button';
import { DateField } from '../../components/date_field';
import { DisplaySize } from '../../display_size';
import { LabeledCheckbox } from '../../components/labeled_checkbox';
import { Modal } from '../../components/modal';
import { RequestSortSelect } from './request_sort_select';

type Type = Nexus.AccountModificationRequest.Type;
const Type = Nexus.AccountModificationRequest.Type;

interface Properties {

  /** The size at which to display the component. */
  displaySize: DisplaySize;

  /** The selected categories. */
  categories: Set<Type>;

  /** The selected start date. */
  startDate?: Beam.Date;

  /** The selected end date. */
  endDate?: Beam.Date;

  /** The selected sort field. */
  sortKey: RequestSortSelect.Field;

  /** Called when the user submits the filter criteria. */
  onSubmit?: (criteria: RequestFilterModal.Criteria) => void;

  /** Called when the modal should be closed. */
  onClose?: () => void;
}

interface State {
  categories: Set<Type>;
  startDate?: Beam.Date;
  endDate?: Beam.Date;
  sortKey: RequestSortSelect.Field;
}

/** A modal/inline panel for filtering requests. */
export class RequestFilterModal extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      categories: new Set(props.categories),
      startDate: props.startDate,
      endDate: props.endDate,
      sortKey: props.sortKey
    };
  }

  public render(): JSX.Element {
    const invalid = this.isInvalid();
    if(this.props.displaySize === DisplaySize.SMALL) {
      return (
        <Modal displaySize={this.props.displaySize}
            title='Filters' onClose={this.props.onClose}>
          <div className={css(STYLES.smallContent)}>
            <div className={css(STYLES.filtersSection)}>
              {this.renderSortBy()}
              <div className={css(STYLES.sectionGap)}/>
              {this.renderCategories()}
              <div className={css(STYLES.sectionGap)}/>
              {this.renderDateCreatedSmall()}
            </div>
            {this.renderErrorSection(invalid)}
            <div className={css(STYLES.actionsSmall)}>
              <Button label='Apply' onClick={this.onSubmit}
                readonly={invalid}/>
            </div>
          </div>
        </Modal>);
    }
    return (
      <Modal displaySize={this.props.displaySize}
          title='Filters' onClose={this.props.onClose}>
        <div className={css(STYLES.largeContent)}>
          <div className={css(STYLES.filtersRow)}>
            {this.renderDateCreatedLarge()}
            <div className={css(STYLES.inlineGap)}/>
            <Button label='Apply' onClick={this.onSubmit}
              readonly={invalid}
              style={STYLE.applyButtonLarge}/>
          </div>
          {this.renderErrorSectionLarge(invalid)}
        </div>
      </Modal>);
  }

  private renderSortBy(): JSX.Element {
    return (
      <div>
        <div className={css(STYLES.fieldRow)}>
          <div className={css(STYLES.fieldLabel)}>Sort by</div>
          <div className={css(STYLES.fieldLabelGap)}/>
          <RequestSortSelect value={this.state.sortKey}
            onChange={this.onSortChange}/>
        </div>
      </div>);
  }

  private renderCategories(): JSX.Element {
    return (
      <div>
        <div className={css(STYLES.legend)}>Categories</div>
        <div className={css(STYLES.legendGap)}/>
        <LabeledCheckbox label='Risk Controls'
          isChecked={this.state.categories.has(Type.RISK)}
          onChange={this.onToggleRisk}/>
        <div className={css(STYLES.checkboxGap)}/>
        <LabeledCheckbox label='Entitlements'
          isChecked={this.state.categories.has(Type.ENTITLEMENTS)}
          onChange={this.onToggleEntitlements}/>
        <div className={css(STYLES.checkboxGap)}/>
        <LabeledCheckbox label='Compliance'
          isChecked={this.state.categories.has(Type.COMPLIANCE)}
          onChange={this.onToggleCompliance}/>
      </div>);
  }

  private renderDateCreatedSmall(): JSX.Element {
    return (
      <div>
        <div className={css(STYLES.legend)}>Date Created</div>
        <div className={css(STYLES.legendGap)}/>
        <div className={css(STYLES.fieldRow)}>
          <div className={css(STYLES.fieldLabel)}>Start</div>
          <div className={css(STYLES.fieldLabelGap)}/>
          <DateField displaySize={DisplaySize.SMALL}
            value={this.state.startDate}
            onChange={this.onStartDateChange}/>
        </div>
        <div className={css(STYLES.dateGap)}/>
        <div className={css(STYLES.fieldRow)}>
          <div className={css(STYLES.fieldLabel)}>End</div>
          <div className={css(STYLES.fieldLabelGap)}/>
          <DateField displaySize={DisplaySize.SMALL}
            value={this.state.endDate}
            onChange={this.onEndDateChange}/>
        </div>
      </div>);
  }

  private renderDateCreatedLarge(): JSX.Element {
    return (
      <div className={css(STYLES.dateCreatedLarge)}>
        <div className={css(STYLES.dateLegendLarge)}>Date Created</div>
        <div className={css(STYLES.dateLegendGapLarge)}/>
        <div className={css(STYLES.dateFieldsRow)}>
          <div className={css(STYLES.dateFieldLabel)}>Start</div>
          <div className={css(STYLES.fieldLabelGap)}/>
          <DateField displaySize={DisplaySize.LARGE}
            value={this.state.startDate}
            onChange={this.onStartDateChange}/>
          <div className={css(STYLES.dateFieldGapLarge)}/>
          <div className={css(STYLES.dateFieldLabel)}>End</div>
          <div className={css(STYLES.fieldLabelGap)}/>
          <DateField displaySize={DisplaySize.LARGE}
            value={this.state.endDate}
            onChange={this.onEndDateChange}/>
        </div>
      </div>);
  }

  private renderErrorSection(invalid: boolean): JSX.Element {
    if(invalid) {
      return (
        <div className={css(STYLES.errorSectionSmall)}>
          <span className={css(STYLES.errorMessage)}>
            End date must be greater than start date</span>
        </div>);
    }
    return <div className={css(STYLES.errorSpacerSmall)}/>;
  }

  private renderErrorSectionLarge(invalid: boolean): JSX.Element {
    if(!invalid) {
      return null;
    }
    return (
      <div className={css(STYLES.errorSectionLarge)}>
        <span className={css(STYLES.errorMessage)}>
          End date must be greater than start date</span>
      </div>);
  }

  private isInvalid(): boolean {
    if(this.state.startDate === undefined ||
        this.state.endDate === undefined) {
      return false;
    }
    return isAfter(this.state.startDate, this.state.endDate);
  }

  private onSortChange = (value: RequestSortSelect.Field) => {
    this.setState({sortKey: value});
  }

  private onToggleRisk = () => {
    this.toggleCategory(Type.RISK);
  }

  private onToggleEntitlements = () => {
    this.toggleCategory(Type.ENTITLEMENTS);
  }

  private onToggleCompliance = () => {
    this.toggleCategory(Type.COMPLIANCE);
  }

  private toggleCategory(type: Type) {
    const next = new Set(this.state.categories);
    if(next.has(type)) {
      next.delete(type);
    } else {
      next.add(type);
    }
    this.setState({categories: next});
  }

  private onStartDateChange = (value: Beam.Date) => {
    this.setState({
      startDate: value,
      endDate: this.state.endDate ?? today()
    });
  }

  private onEndDateChange = (value: Beam.Date) => {
    this.setState({
      startDate: this.state.startDate ?? today(),
      endDate: value
    });
  }

  private onSubmit = () => {
    if(this.isInvalid()) {
      return;
    }
    this.props.onSubmit?.({
      categories: new Set(this.state.categories),
      startDate: this.state.startDate,
      endDate: this.state.endDate,
      sortKey: this.state.sortKey
    });
  }
}

export namespace RequestFilterModal {

  /** The filter criteria submitted by the modal. */
  export interface Criteria {

    /** The selected categories. */
    categories: Set<Nexus.AccountModificationRequest.Type>;

    /** The selected start date. */
    startDate?: Beam.Date;

    /** The selected end date. */
    endDate?: Beam.Date;

    /** The selected sort field. */
    sortKey: RequestSortSelect.Field;
  }
}

function today(): Beam.Date {
  const now = new Date();
  return new Beam.Date(now.getFullYear(), now.getMonth() + 1, now.getDate());
}

function isAfter(a: Beam.Date, b: Beam.Date): boolean {
  if(a.year !== b.year) {
    return a.year > b.year;
  }
  if(a.month !== b.month) {
    return a.month > b.month;
  }
  return a.day > b.day;
}

const STYLE = {
  applyButtonLarge: {
    width: '140px',
    flexShrink: 0
  } as React.CSSProperties
};

const STYLES = StyleSheet.create({
  smallContent: {
    display: 'flex',
    flexDirection: 'column',
    flexGrow: 1,
    overflow: 'hidden'
  },
  filtersSection: {
    flexGrow: 1,
    overflowY: 'auto',
    padding: '18px 18px 18px'
  },
  actionsSmall: {
    flexShrink: 0,
    backgroundColor: '#FFFFFF',
    padding: '18px 18px 30px',
    borderTop: '1px solid #E6E6E6',
    display: 'flex',
    justifyContent: 'center'
  },
  largeContent: {
    padding: '18px 18px 18px'
  },
  filtersRow: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'flex-end'
  },
  inlineGap: {
    width: '30px',
    flexShrink: 0
  },
  sectionGap: {
    height: '30px'
  },
  legend: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#333333'
  },
  legendGap: {
    height: '18px'
  },
  fieldRow: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center',
    height: '34px'
  },
  fieldLabel: {
    width: '3.375rem',
    flexShrink: 0,
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    color: '#333333'
  },
  fieldLabelGap: {
    width: '8px',
    flexShrink: 0
  },
  checkboxGap: {
    height: '10px'
  },
  dateGap: {
    height: '10px'
  },
  dateCreatedLarge: {
    flexGrow: 1
  },
  dateLegendLarge: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    fontWeight: 500,
    color: '#333333'
  },
  dateLegendGapLarge: {
    height: '12px'
  },
  dateFieldsRow: {
    display: 'flex',
    flexDirection: 'row',
    alignItems: 'center'
  },
  dateFieldLabel: {
    width: '2.5rem',
    flexShrink: 0,
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    color: '#333333'
  },
  dateFieldGapLarge: {
    width: '18px',
    flexShrink: 0
  },
  errorSectionSmall: {
    padding: '18px 18px 0'
  },
  errorSpacerSmall: {
    flexGrow: 1
  },
  errorSectionLarge: {
    paddingTop: '18px'
  },
  errorMessage: {
    fontFamily: 'Roboto',
    fontSize: '0.875rem',
    color: '#E63F44'
  }
});
