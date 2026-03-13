import * as React from 'react';
import { Select } from '../..';
import { RequestsModel } from './requests_model';

interface Properties extends
    Omit<React.ComponentProps<typeof Select>, 'onChange' | 'value'> {

  /** The currently selected sort field. */
  value: RequestsModel.SortField;

  /** Called when the selected sort field changes. */
  onChange?: (value: RequestsModel.SortField) => void;
}

/** A dropdown select for sorting requests. */
export function RequestSortSelect(
    {value, onChange, ...rest}: Properties): JSX.Element {
  const onSelectChange = (selectValue: string) => {
    onChange?.(Number(selectValue) as RequestsModel.SortField);
  };
  return (
    <Select {...rest} value={value.toString()}
        style={{...STYLE.select, ...rest.style}}
        onChange={onSelectChange}>
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
    </Select>);
}

const STYLE = {
  select: {
    width: '100%',
    cursor: 'pointer'
  } as React.CSSProperties
};
