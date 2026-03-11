import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { RequestsModel } from './requests_model';

type SelectAttributes = Omit<
  React.SelectHTMLAttributes<HTMLSelectElement>, 'onChange'>;

interface Properties extends SelectAttributes {

  /** The currently selected sort field. */
  value: RequestsModel.SortField;

  /** Called when the selected sort field changes. */
  onChange?: (value: RequestsModel.SortField) => void;
}

/** A dropdown select for sorting requests. */
export class RequestSortSelect extends React.Component<Properties> {
  public render(): JSX.Element {
    const { value, onChange, ...selectProps } = this.props;
    return (
      <select {...selectProps} value={value}
          onChange={this.onSelectChange}
          className={css(STYLES.select)}>
        <option value={RequestsModel.SortField.LAST_UPDATED}>
          Last Updated</option>
        <option value={RequestsModel.SortField.CREATED}>
          Created</option>
        <option value={RequestsModel.SortField.ACCOUNT}>
          Account</option>
        <option value={RequestsModel.SortField.REQUESTER}>
          Requester</option>
        <option value={RequestsModel.SortField.EFFECTIVE_DATE}>
          Effective Date</option>
      </select>);
  }

  private onSelectChange = (event: React.ChangeEvent<HTMLSelectElement>) => {
    this.props.onChange?.(
      Number(event.target.value) as RequestsModel.SortField);
  };
}

const STYLES = StyleSheet.create({
  select: {
    boxSizing: 'border-box',
    width: '100%',
    height: '34px',
    backgroundColor: '#F8F8F8',
    border: '1px solid #C8C8C8',
    borderRadius: '1px',
    color: '#333333',
    fontSize: '14px',
    fontFamily: 'Roboto',
    paddingLeft: '10px',
    paddingRight: '28px',
    appearance: 'none',
    WebkitAppearance: 'none',
    MozAppearance: 'none',
    backgroundImage: `url("data:image/svg+xml,${encodeURIComponent(
      '<svg xmlns="http://www.w3.org/2000/svg" width="8" height="6" ' +
      'viewBox="0 0 8 6"><path d="M7.1,0,4,4.1.9,0,0,.7,4,6,8,.7Z" ' +
      'fill="#333"/></svg>'
    )}")`,
    backgroundRepeat: 'no-repeat',
    backgroundPosition: 'right 10px center',
    cursor: 'pointer',
    outline: 'none',
    ':focus': {
      borderColor: '#684BC7'
    }
  }
});
