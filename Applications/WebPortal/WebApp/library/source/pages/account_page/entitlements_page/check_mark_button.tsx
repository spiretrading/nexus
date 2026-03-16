import * as React from 'react';

interface Properties {

  /** The width and height of the button. */
  size: number | string;

  /** Determines if the checkmark is currently active (green) or inactive (grey). */
  isChecked: boolean;

  /** The onClick event handler. */
  onClick?: () => void;
}

/** Displays a check mark button. */
export function CheckMarkButton(props: Properties): JSX.Element {
  const source = props.isChecked ?
    'resources/account_page/entitlements_page/icons/check-green.svg' :
    'resources/account_page/entitlements_page/icons/check-grey.svg';
  return (
    <div onClick={props.onClick} style={STYLE.wrapper}>
      <img src={source} alt='checkbox'
        height={props.size} width={props.size}/>
    </div>);
}

const STYLE: Record<string, React.CSSProperties> = {
  wrapper: {
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    width: '20px',
    height: '20px',
    cursor: 'pointer'
  }
};
